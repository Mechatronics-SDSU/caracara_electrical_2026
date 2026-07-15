#Engineer: Deepankar Tiwari
#SDSU Mechatronics EE, RoboSub 2026

from machine import ADC, Pin, I2C
from utime import sleep, sleep_ms
#Note: Temperature is needed because the pressure sensor's silicon changes characteristics as temperature changes.
#      Hence, the reading is used to correctly recalibrate pressure based on the sensor becoming hotter vs colder.

# -------------------------------------------------
# Battery ADC settings
# -------------------------------------------------
ADC_PIN = 26 #Connection to GPIO26 (ADC0 on Pico)
ADC_MAX = 65535.0 #MircoPython converts 12 bits to 16 bits, .0 for float values
V_REF = 3.283 #[Volts], Pico IO voltage is 3.3V, chose lower value as reference

R1 = 100000.0 #[Ohms], Top resistor = 100k Ohms
R2 = 22000.0 #[Ohms], Bottom resistor = 22k Ohms
DIVIDER_SCALE = (R1 + R2) / R2 #Voltage Divider Scaling Factor, inverse is scaling factor of voltage divider reading
REPORT_PERIOD_SECOND = 1 #Updates output every second

M = 0.995 #Linear regression slope
B = 0.166071 #Linear regression y intercept
PICO_OFFSET = -0.07 #Internal Pico offset

adc = ADC(ADC_PIN) #Initializing the ADC pin

#Function to read voltage
def read_battery_voltage():
    v_adc = float(adc.read_u16() / ADC_MAX) * V_REF #calculate adc voltage
    v_bat = v_adc * DIVIDER_SCALE #upscale divided voltage to original value
    v_corrected = (v_bat - B) / M + PICO_OFFSET #Corrected reading based on linear regression
    return v_corrected #Returns corrected battery voltage


# -------------------------------------------------
# Bar02 / MS5837 settings
# -------------------------------------------------
MS5837_ADDR = 0x76 # Bar02's (I^2)(C) address, 118 in Hex

CMD_RESET = 0x1E # Resets sensor
CMD_ADC_READ = 0x00 # Resets sensor's ADC after pressure or temperature conversion
CMD_PROM_READ = 0xA0 # Base command to read sensor calibrated values, PROM = programmable read only memory
CMD_CONVERT_D1_8192 = 0x4A  # High-res pressure conversion, D1 is raw pressure
CMD_CONVERT_D2_8192 = 0x5A  # High-res temperature conversion, D1 is raw pressure

FLUID_DENSITY = 997.0       # Freshwater in kg/m^3
GRAVITY = 9.80665 # Acceleration due to gravity on Earth

# Creates (I^2)(C) connection: GP18 = SDA, GP19 = SCL, Bus 1 for (I^2)(C), 50 kHz clk frequency
# SDA = Serial Data, SCL = Serial Clock
# (I^2)(C) = Inter-Integrated Circuit Protocol
# Allows communication between chips (peripheral and controller)
i2c = I2C(1, sda=Pin(18), scl=Pin(19), freq=50000)

C = [0] * 8 #List for calibration coefficients; [0,0,0,0,0,0,0,0]
pressure_mbar = 0.0 # Initial pressure, millibars
temperature_c = 0.0 # Initial temperature, Celsius
surface_pressure_mbar = 1013.25 # 1 atm or 1013.25 millibars

#Prepares Bar02 depth sensor for use
def bar02_init():
    global C # Initilizes C as a global variable

    print("Resetting Bar02...")
    i2c.writeto(MS5837_ADDR, bytes([CMD_RESET])) # Resets sensor
    sleep_ms(500) # Wait 500 milliseconds for reset

    print("After reset scan:", i2c.scan()) # Checks if sensor is still exists on (I^2)(C) Bus

    for i in range(7):  # Read C0 through C6 only, C7 throws error
        cmd = CMD_PROM_READ + i * 2 # Creates addresses
        i2c.writeto(MS5837_ADDR, bytes([cmd])) # Writes to created addresses
        sleep_ms(20) # Delay of 20 ms before reading response
        data = i2c.readfrom(MS5837_ADDR, 2) # Read 2 bytes from the sensor PROM (16 bits, 2 bytes)
        C[i] = (data[0] << 8) | data[1] # Combine two bytes into 1, 16-bit number

    print("Calibration coefficients:", C) # Calibration coefficients unique to every Bar02

# Helper function to read the 24 bit ADC
def bar02_read_adc():
    i2c.writeto(MS5837_ADDR, bytes([CMD_ADC_READ])) # Read last conversion result
    data = i2c.readfrom(MS5837_ADDR, 3) # Read 3 bytes of data from ADC
    return (data[0] << 16) | (data[1] << 8) | data[2] # Combine 3 bytes into one 24-bit raw value

# Actual function to read ADC
def bar02_read():
    global pressure_mbar, temperature_c # Allows updating of previously defined global variables (not local var)

    i2c.writeto(MS5837_ADDR, bytes([CMD_CONVERT_D1_8192])) # Start pressure conversion
    sleep_ms(20) # Wait 20ms for conversion to finish
    D1 = bar02_read_adc() # Reads raw pressure value

    i2c.writeto(MS5837_ADDR, bytes([CMD_CONVERT_D2_8192])) # Start temperature conversion
    sleep_ms(20) # Wait Wait 20ms for conversion to finish; second 20 ms to complete 40ms read
    D2 = bar02_read_adc() # Read raw temperature value

    # All calculations come from MS5837-02BA datasheet
    dT = D2 - C[5] * 256 # Difference between reading and calibration, 256 is scaling factor to convert to raw ADC
    TEMP = 2000 + dT * C[6] / 8388608 #Compensated temp to 100th of a degree in Celsius, Fixed point arithmetic; 2000 is 20.00C

    # MS5837-02BA / Bar02 compensation
    SENS = C[1] * 65536 + (C[3] * dT) / 128 # Pressure sensitivty compensation, Fixed point arithmetic, C1 (base pressure sensitivity) depends on C2 (temp)
    OFF = C[2] * 131072 + (C[4] * dT) / 64 # Pressure offset compensation, Corrects sensor's pressure zero point
    P = (D1 * SENS / 2097152 - OFF) / 32768 # Pressure using raw pressure plus colibration compensation, Scaling factors due to fixed point arithmetic

    temperature_c = TEMP / 100.0 # Converts from 100ths of C to C
    pressure_mbar = P / 100.0 # Converts from 0.01 millibars to millibars

# Zeroes depth reading 
def set_surface_pressure():
    global surface_pressure_mbar # Access global variable
    bar02_read() # Take temperature and pressure reading, store only pressure reading
    surface_pressure_mbar = pressure_mbar # Use value stored in global pressure variable

# Returns depth in meters
def get_depth_m():
    pressure_pa = (pressure_mbar - surface_pressure_mbar) * 100.0 #Delta pressure in Pa, 1 mbar = 100 Pa
    return pressure_pa / (FLUID_DENSITY * GRAVITY) # Uses hydrostatic pressure equation 
# Note: Hydrostatic equations only apply if sub does not change vertical position quickly

# -------------------------------------------------
# Main program
# -------------------------------------------------
# Calibration of low pass filtering constants and sensor
depth_filtered = None # Used to initilize variable to take initial reading
alpha = 0.15 # Low-pass filter depth strength
sensor_has_reading = False # Initially no reading


print("Scanning I2C...")
print(i2c.scan()) # Checks which (I^2)(C) are connected 

try:
    bar02_init() #Resets the sensor and reads its calibration values

    print("Calibrating surface pressure...")
    set_surface_pressure() # Sets current pressure as zero depth 
    print("Surface pressure: {:.2f} mbar".format(surface_pressure_mbar)) # Prints zero-depth reference pressure

    sensor_has_reading = True

except OSError as e:
    print("Bar02 sensor init falied: ", e)

while True:

    battery_v = read_battery_voltage() # Reads voltage from GP26
    
    if sensor_has_reading:
        try:
            bar02_read() # Read both temperature and pressure values 
            depth_raw = get_depth_m() # Pressure to raw depth

            # Low pass filter (First order): (1- alpha)% old value + (alpha)% new value (Exponential moving average or EMA)
            # First order because its a mathematical model which contains only one previous state
            if depth_filtered is None: # (== not used, keyword "is" used instead)
                depth_filtered = depth_raw # Intitial reading taken as starting point for filtering
            else:
                depth_filtered = (1 - alpha) * depth_filtered + alpha * depth_raw # Low pass filter

            print("BAT:{:.2f},DEPTH:{:.3f},PRESS:{:.2f},TEMP:{:.2f}".format(
                battery_v,
                depth_filtered,
                pressure_mbar,
                temperature_c
            ))
        
        except OSError as e:
            print("Bar02 read failed: ", e)
            sensor_has_reading = False

            print("BAT:{:.2f},DEPTH:NaN,PRESS:NaN,TEMP:NaN".format(battery_v))
    else:
        print("BAT:{:.2f},DEPTH:NaN,PRESS:NaN,TEMP:NaN".format(battery_v))
    
    sleep(REPORT_PERIOD_SECOND) # Wait before taking next reading