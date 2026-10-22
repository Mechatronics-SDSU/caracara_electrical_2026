Author: Trevor Young July 7, 2026 

# **STM32 Firmware Architecture Redesign - Implementation Summary** 

## **Overview** 

To note - Aspects of this readme are unreliable. This was written before implementation, and was not updated after the systems outlined in this document were altered slightly. I say this to convey that, while this document does largely represent the logical flow and high level functionality of the system, this document is not gospel. 

This document summarizes the redesigned STM32 firmware architecture for the submarine's multi-subsystem embedded control application. It follows on from the original "STM32 Firmware Architecture Review and Redesign" document (May 26, 2026), which identified a monolithic main.c , an untyped shared USB buffer, distributed subsystem updates, and application logic embedded in interrupt handlers as the primary maintainability risks in the original firmware. 

This redesign replaces the shared usbBuffer array with a typed SystemState struct, consolidates kill-state/safety logic into a dedicated module, moves motor (and payload actuator) control onto explicit state machines driven from the main loop instead of timer interrupt callbacks, and adds two new subsystems (torpedo and grabber) alongside the previously-existing dropper. 

## **New High-Level Data Flow** 

USB host (Orin) | v CDC_Receive_FS()  [usbd_cdc_if.c, runs in USB interrupt context] | v 

SystemState_UpdateFromUSB(&pendingState, Buf, len)  [system_state.c] | v pendingState  (private staging struct, written only by the USB ISR) | |   stateUpdatePending = 1 v SystemState_ApplyPendingUpdate(&state)  [system_state.c, called from main loop] |   -- short __disable_irq()/__enable_irq() critical section -- |   copies motorValues, killState, powerOff, servo1, servo2, |   dropper, torpedo from pendingState into state, then clears flag v 

**Outdated:** I believe that this struct is no longer accurate to what is reflected in the code. While I am making this comment from memory four months after the fact, I think the struct size increased from 48 bytes to 52 or 56 - and I also think that some of the variables listed in this struct were replaced with other, more important data. 

SystemState *state  (single global struct, defined in system_state.h) | 

- |-- motorValues[0..7]      -> motor_control.c 

- |-- killState / extKill    -> safety_manager.c 

- |-- servo1, servo2         -> grabber.c 

- |-- dropper                -> dropper.c 

- |-- torpedo                -> torpedo.c 

- |-- powerOff                -> power_manager.c 

- |-- usbConnected            -> rgb_led.c 

- |-- motorsArming            -> rgb_led.c 

- |-- depth                   -> depth_sensor.c (written) / transmitted back over USB 

- |-- greenPressed            -> button_state.c (written) / depth_sensor.c (transmitted) 

main.c main loop (polled, no application logic in ISRs except USB reception): SystemState_ApplyPendingUpdate -> USB_Manager_Update -> RGB_LED_Update 

- -> Safety_Manager_Update -> Motor_Update -> Dropper_Update -> Torpedo_Update 

- -> Grabber_Update -> Button_Update -> Power_Down -> Depth_Sensor_Update 

The USB buffer is now parsed into a single named struct instead of subsystems each reading raw indices out of a shared array. Subsystems still read/write the same struct instance, but field names replace magic offsets, and update ordering is explicit and linear in main.c rather than distributed across interrupt callbacks and USB event handlers. 

SystemState_ApplyPendingUpdate() runs first in the loop specifically so every subsystem below it sees a consistent, fully-applied snapshot of the latest USB packet for that iteration. 

## **General Architecture** 

The redesigned implementation has the following properties: 

- **Typed shared state.** The SystemState struct ( system_state.h ) replaces the untyped usbBuffer[16] array as the primary data-sharing mechanism between subsystems. Fields are named (e.g. motorValues[8] , killState , servo1 ) rather than accessed by numeric index. 

- **Centralized USB parsing.** SystemState_UpdateFromUSB() is the single function responsible for copying USB packet bytes into a struct, with an explicit length check ( USB_PACKET_MIN_LEN ) before any copy occurs. Subsystems no longer parse the USB buffer themselves. 

- **Atomic USB-to-state handoff.** USB-sourced fields are parsed into a private staging struct ( pendingState ) inside the USB ISR, then applied to the live state struct in one short critical section from the main loop ( SystemState_ApplyPendingUpdate() ). No subsystem ever observes a partially-updated set of USB-sourced fields. 

- **Per-subsystem state machines.** Motor, torpedo, dropper, and (presumably) grabber control now follow a common _Init / _Update / _Start_Arming / _Kill 

pattern built around HAL_GetTick() -based timing rather than blocking delays or interrupt-driven timing. 

- **Consolidated kill-state handling.** safety_manager.c centralizes internal (USB-commanded) and external (button-commanded) kill-state logic, including edge detection for kill and un-kill transitions, in one file — replacing the original's kill logic that was spread across main.c , usb_data.c , and interface.c . 

- **Application logic removed from most interrupt handlers.** stm32g4xx_it.c now contains only HAL-standard dispatch ( HAL_PCD_IRQHandler , HAL_UART_IRQHandler , HAL_TIM_IRQHandler , HAL_IncTick ) with no application-level state changes inside — all subsystem updates happen in the polled main loop. The one exception is USB reception, discussed below. 

- **main.c as orchestrator, not implementer.** main.c performs HAL/peripheral init, subsystem _Init calls, and then runs a flat, linearly-ordered polling loop calling each subsystem's _Update() function. It no longer contains sensor state-machine logic, motor safety checks, or USB parsing directly. 

## **Outline of Important Files - firmware only** 

- **main.c** - entry point; initializes HAL, clocks, GPIO, timers, UART, USB, and I2C; calls subsystem _Init functions; runs the main polling loop. 

- **system_state.c / system_state.h** - defines the SystemState struct, SystemState_UpdateFromUSB() (parses an incoming USB packet into a struct), the pendingState staging struct, and SystemState_ApplyPendingUpdate() (atomically applies USB-sourced fields from pendingState into the live state struct). 

- **usbd_cdc_if.c** - USB CDC class callbacks (ST HAL-generated); CDC_Receive_FS() parses incoming USB data directly into pendingState via SystemState_UpdateFromUSB() and sets stateUpdatePending . No longer touches the live state struct directly. 

- **usb_manager.c** - polls VBUS presence, drives the D+ pull-up line, and sets state->usbConnected . Does not parse USB data (that responsibility now lives in system_state.c / usbd_cdc_if.c ). 

- **safety_manager.c** - reads the external kill GPIO, reconciles it with the USB-commanded internal kill state, drives the internal kill output pin, and calls _Kill() / _Start_Arming() on the motor and dropper subsystems on kill/un-kill transitions. 

- **motor_control.c** - drives 8 ESC-controlled motors via PWM (TIM1, 2, 3, 4, 5, 15, 16, 17) through an arming/ready/killed state machine. 

- **torpedo.c** - servo-driven torpedo firing mechanism; follows the same 

   - _Init/_Update/_Kill/_Start_Arming pattern as motor_control.c 

- **dropper.c** - servo-driven ball-dropper mechanism; same pattern as motor_control.c . Referenced directly by safety_manager.c ( Dropper_Kill , Dropper_Start_Arming ). 

- **grabber.c** - drives two servos ( servo1 , servo2 , wired to J5 rail pins 30 and 29 respectively) to open/close and move a mechanical arm; same pattern as motor_control.c . 

- **depth_sensor.c** - MS5837 pressure/depth sensor driver; runs a non-blocking state machine over I2C to read pressure and temperature, computes depth, and transmits a fixed 10-byte status packet back over USB CDC. 

- **power_manager.c** - handles internal (USB-commanded) and external (button-commanded) power-down. 

- **rgb_led.c** - drives external status LED: white during initialization, blinking blue while motors are arming, solid green when USB is connected, solid red when USB is disconnected. 

- **button_state.c** - handles the external power button; interrupt/event-driven rather than state-machine-polled, unlike the other subsystems. 

- **gpio.c** - MX_GPIO_Init() only; enables GPIO port clocks (A, B, C, F). Per-pin configuration is presumably handled elsewhere (likely stm32g4xx_hal_msp.c / CubeMX-generated init inside each peripheral's own _Init ). 

- **stm32g4xx_it.c** - IRQ handlers; dispatches to HAL handlers only ( HAL_PCD_IRQHandler , HAL_UART_IRQHandler , HAL_TIM_IRQHandler for TIM6/TIM7, HAL_IncTick ). No application logic present. 

- **stm32g4xx_hal_msp.c** - peripheral clock/GPIO alternate-function init (ST HAL-generated) 

- **system_stm32g4xx.c / syscalls.c / sysmem.c** - standard startup/runtime support files (ST/newlib-generated). Not application-specific; not reviewed in detail. 

## **Logical Flow** 

main.c | HAL_Init() SystemClock_Config() | MX_GPIO_Init() MX_TIM1_Init() ... MX_TIM20_Init() MX_USB_Device_Init() MX_USART1_UART_Init() MX_TIM6_Init() MX_TIM7_Init() MX_I2C1_Init() | Motor_Init(&state)      // starts PWMs for motors, begins arming state machine 

Torpedo_Init(&state) Dropper_Init(&state) Grabber_Init(&state) 

| 

Reset external LEDs (blue/green/red pins low) HAL_TIM_Base_Start_IT(&htim6) 

Depth_Sensor_Init(&hi2c1, &state)   // resets sensor, reads + CRC-checks PROM coefficients 

| 

while(true) 

SystemState_ApplyPendingUpdate(&state)  // atomically apply latest USB packet, if any USB_Manager_Update(&state)      // VBUS presence, pull-up control RGB_LED_Update(&state)          // cosmetic, reflects usbConnected / motorsArming Safety_Manager_Update(&state)   // kill-state reconciliation, drives Motor_Kill/Dropper_Kill Motor_Update()                  // apply commanded motor values (if READY and not killed) Dropper_Update() Torpedo_Update() Grabber_Update() Button_Update(&state)           // external power button Power_Down(&state)              // power off if flagged by button or USB command Depth_Sensor_Update()           // non-blocking I2C state machine, transmits packet on completion 

USB reception itself is not polled in the main loop — it happens asynchronously in CDC_Receive_FS() , invoked from USB interrupt context whenever the host sends a packet. As of this revision, the ISR no longer writes directly into the live SystemState struct; it writes into a private staging struct ( pendingState ) and sets a flag, which SystemState_ApplyPendingUpdate() consumes atomically at a fixed point in the main loop. 

## **Core Functionality** 

### **1. System State / USB Communication** 

#### **Responsibilities** 

- Receive USB packets from the host (Orin) and parse them into a typed struct 

- Report connection presence and control the D+ pull-up 

- Transmit telemetry (currently depth + button/kill status) back over USB 

#### **Functions** 

- SystemState_UpdateFromUSB(SystemState *state, const uint8_t *buffer, uint8_t len) — system_state.c ; parses a raw USB packet into a SystemState (called with &pendingState from the USB ISR) 

- SystemState_ApplyPendingUpdate(SystemState *state) — system_state.c ; atomically copies USB-sourced fields from pendingState into the live struct, called once per main loop iteration 

- USB_Manager_Update(SystemState *state) — usb_manager.c 

- CDC_Receive_FS(...) , CDC_Transmit_FS(...) — usbd_cdc_if.c 

**USB packet layout** (all fields 4 bytes, little-endian, offsets 0-52): 

motorValues[0..7] (0-28), killState (32), powerOff (36), servo1 (40), servo2 (44), dropper (48), torpedo (52). 

**Struct fields not sourced from USB:** intKillState , extKillState , powerDownButton , usbConnected , motorsArming , depth , greenPressed — these are written by other subsystems ( safety_manager.c , usb_manager.c , motor_control.c , depth_sensor.c , button_state.c respectively) and read back out for telemetry or LED behavior. 

**Improvement over original** : subsystems now consume named struct fields instead of pulling from a raw indexed array, and there is a single, centralized parse function with a minimum-length check, versus the original's scattered per-subsystem reads of usbBuffer . As of this revision, the legacy usbBuffer[16] intermediate array has been removed entirely — SystemState_UpdateFromUSB() parses directly from the USB driver's Buf — and the parse-then-apply split ( pendingState in the ISR, atomic apply in the main loop) means USB-sourced fields are never read by another subsystem in a partially-updated state. 

### **2. Motor Control** 

#### **Responsibilities** 

- Drive 8 ESC-controlled motors via PWM (TIM1, TIM2, TIM3, TIM4, TIM5, TIM15, TIM16, TIM17) 

- Support positive and negative (forward/reverse) command values with a defined deadband 

- Run a startup arming sequence before accepting commands 

- Clamp out-of-range values to neutral 

**Functions** ( motor_control.c ) 

- Motor_Init(SystemState *state) — stores the state pointer, starts all motor PWM channels, begins arming 

- Motor_Update(void) — state machine: MOTOR_STATE_ARMING -> MOTOR_STATE_READY , or MOTOR_STATE_KILLED holds all motors at neutral 

- Motor_Start_Arming(void) — resets motors to neutral, restarts the arming timer, sets motorsArming for the LED subsystem 

- Motor_Kill(void) — forces the state machine to MOTOR_STATE_KILLED and neutrals all motors immediately 

**Improvement over original** : safety-relevant motor behavior (arming delay, out-of-range clamping, neutral-on-kill) is now consolidated in this one file and driven by HAL_GetTick() polling rather than being split across main.c and timer interrupt callbacks. 

### **3. Payload Actuators (Torpedo, Dropper, Grabber)** 

#### **Responsibilities** 

- dropper.c — drives a servo to release a ball 

- torpedo.c — drives a servo to fire a torpedo 

- grabber.c — drives two servos ( servo1 on J5 pin 30, servo2 on J5 pin 29) to move and open/close a mechanical arm 

#### **Functions** 

All three are described as following the same _Init / _Update / _Start_Arming / _Kill pattern as motor_control.c . Confirmed call sites from main.c and safety_manager.c : 

- Torpedo_Init , Torpedo_Update — called from main.c 

- Dropper_Init , Dropper_Update , Dropper_Kill , Dropper_Start_Arming — called from main.c and safety_manager.c 

- Grabber_Init , Grabber_Update — called from main.c 

**Concern:** Torpedo_Kill and Grabber_Kill are not called anywhere in safety_manager.c . On a kill event, Motor_Kill() and Dropper_Kill() are invoked, but torpedo and grabber are not. These two subsystems have not yet been wired into the kill/safety path. This means an internal or external kill event currently stops the motors and dropper but does not neutral the torpedo or grabber, which is not the biggest issue, but it is worth noting. 

### **4. Kill-State / Safety System** 

#### **Responsibilities** 

- Track internal (USB/Orin-commanded) and external (button-commanded) kill state 

- Drive the internal kill GPIO output 

- Detect kill and un-kill edges and propagate them to dependent subsystems 

**Functions** ( safety_manager.c ) 

- Safety_Manager_Update(SystemState *state) — the sole entry point, called once per main loop iteration 

#### **Logic, as implemented:** 

- Reads EXT_KILL_GPIO_Port / EXT_KILL_Pin into state->extKillState every call 

- If state->killState (USB-commanded) is set: drives INT_KILL_Pin high, sets state->intKillState = 1 , and calls Motor_Kill() + Dropper_Kill() 

- Otherwise: drives INT_KILL_Pin low, sets state->intKillState = 0 

- On a rising edge of state->extKillState (external button kill): also calls Motor_Kill() + Dropper_Kill() 

- On a falling edge of either kill source (transitioning from previously-killed to both-clear): calls Motor_Start_Arming() + Dropper_Start_Arming() to re-arm 

**Improvement over original** : kill logic that was previously spread across main.c , Set_TIM_Compare() , HAL_TIM_PeriodElapsedCallback() , usb_data.c , and interface.c is now in one file with one update function and explicit edge-detected state transitions.  

### **5. Depth Sensor System** 

#### **Responsibilities** 

- Interface with the MS5837 pressure/depth sensor over I2C 

- Run a non-blocking conversion state machine to read pressure and temperature ● Apply first- and second-order MS5837 compensation and compute depth 

- Transmit a fixed-format status packet back over USB 

**Functions** ( depth_sensor.c ) 

- Depth_Sensor_Init(I2C_HandleTypeDef *hi2c, SystemState *state) — resets the sensor, reads all 7 PROM calibration words, validates them with a CRC-4 

check against the datasheet algorithm, returns HAL_ERROR on any I2C failure or CRC mismatch 

- Depth_Sensor_Update(void) — 3-state machine ( start D1 conversion -> read D1 / start D2 -> read D2, compute, transmit ) driven by HAL_GetTick() against an 18 ms OSR-8192 conversion time 

- compute_depth(void) (static) — applies MS5837 compensation math, including the low-temperature second-order correction for readings under 20°C, and writes the result to state->depth 

- transmit_packet(void) (static) — builds and sends a 10-byte packet (header, greenPressed , extKillState , intKillState , 4-byte float depth, CRLF) over USB CDC 

**Improvement over original** : initialization now explicitly checks HAL_StatusTypeDef on every I2C transaction and validates PROM data via CRC before accepting it, which directly answers two of the original document's open questions ("what if I2C fails," "what if PROM calibration data is invalid") for the init path. 

#### **Still open — the original document's other questions remain unanswered for runtime** 

**reads:** none of the I2C calls inside Depth_Sensor_Update() 's state machine ( case 0 , case 1 , case 2 ) check their HAL_StatusTypeDef return values. A mid-operation I2C failure during normal running would go undetected, and compute_depth() would run on stale or garbage D1 / D2 values, transmitting a bad depth reading over USB with no indication anything went wrong. There is no retry logic and no fallback/safe-mode behavior on a runtime read failure

### **6. Interface / LED / Button System** 

#### **Responsibilities** 

- rgb_led.c — drives status LEDs reflecting system state: white (initializing), blinking blue (motors arming), solid green (USB connected), solid red (USB disconnected) 

- button_state.c — handles the external power button, used to turn the sub on/off 

#### **Functions** 

- RGB_LED_Update(SystemState *state) — called each main loop iteration 

- Button_Update(SystemState *state) — called each main loop iteration; internally event/interrupt-driven per the button's own description, rather than a polled state machine like the other subsystems 

**Improvement over original** : LED behavior is now driven from state fields ( usbConnected , motorsArming ) set by other subsystems, rather than from ad hoc globals ( color , lockColor , checkRedMode , etc.) mutated inside timer interrupt callbacks as in the original design. stm32g4xx_it.c confirms no LED or button logic remains inside HAL_TIM_PeriodElapsedCallback or any other ISR — TIM6 interrupts dispatch only to the HAL library handler. 

**Current Issue** : If memory serves, there is an issue with the button LED. I believe if the button turns green(?) it will not change back to any other color, and will get stuck in that state. There is some issue in the logic for the button system when it detects a press, and it will get stuck on one of the colors. Cant really remember the details, but youll be able to identify the problem fairly quickly. Good luck fixing it. 

### **7. Power Management** 

#### **Responsibilities** 

- Power the system down on either an internal (USB/Orin-commanded, state->powerOff ) or external (button-commanded, state->powerDownButton ) trigger 

#### **Functions** 

- Power_Down(SystemState *state) — called each main loop iteration, after Button_Update 

## **Outstanding Concerns** 

**1. Torpedo and grabber are not wired into the safety system.** Safety_Manager_Update() calls Motor_Kill() / Motor_Start_Arming() and Dropper_Kill() / Dropper_Start_Arming() only. A kill event does not stop the torpedo or grabber. Confirmed directly — not yet implemented. This is the highest-priority remaining item, as it is a live safety gap. 

**2. Depth sensor runtime error handling is incomplete.** Depth_Sensor_Init() checks HAL status and CRC-validates PROM data, but Depth_Sensor_Update() 's I2C calls are unchecked. A runtime I2C failure produces a silently bad depth transmission with no retry or fallback behavior. 

