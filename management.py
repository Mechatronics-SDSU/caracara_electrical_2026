#Engineer: Deepankar Tiwari
#SDSU Mechatronics, Electrical Subteam
#Description: This is the code flashed on the Raspberry Pi Pico to read battery voltage
#             and send the data over through USB. 
#Note: the transmission handling is usb_battery_receiver 
#      and actually reading the voltage is read_voltage

#Importing modules: machine (class that has functions for hardware), ADC (analog to digital converter), time (for waiting + delays)
from machine import ADC
import time

#----------------------------------------------
# Values for Pico Hardware
#----------------------------------------------

ADC_PIN = 26    #Connection to GPIO26 (ADC0 on Pico)
ADC_MAX = 65535.0 #MircoPython converts 12 bits to 16 bits, .0 for float values
V_REF = 3.283     #[Volts], Pico IO voltage is 3.3V, chose lower value as reference
                  

#----------------------------------------------
# Values for Voltage divider
#----------------------------------------------

R1 = 100000.0   #[Ohms], Top resistor = 100k Ohms
R2 = 22000.0   #[Ohms], Bottom resistor = 22k Ohms

DIVIDER_SCALE = 1 #(R1+R2)/(R2)  #Voltage Divider Scaling Factor, inverse is scaling factor of voltage divider reading
REPORT_PERIOD_MS = 1000 #Updates output every half-second, 1s

#Initializing ADC
adc = ADC(ADC_PIN)

while True:
    v_adc: float =  float(adc.read_u16() / ADC_MAX) * V_REF # calculate adc voltage
    v_bat: float = v_adc * DIVIDER_SCALE # upscale divided voltage to original value
    

    print("{:.2f}".format(v_bat)) #Output result via USB (serial)
    time.sleep_ms(REPORT_PERIOD_MS) #Wait half second before next measurement