#Engineer: Deepankar Tiwari
#SDSU Mechatronics, Electrical Subteam
#Description: This is the code flashed on the Raspberry Pi Pico to read battery voltage
#             and send the data over through USB. 
#Note: The transmission handling is usb_battery_receiver 
#      and actually reading the voltage is read_voltage

#Importing modules: machine (class that has functions for hardware), ADC (analog to digital converter), time (for waiting + delays)
from machine import ADC
from utime import sleep

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

DIVIDER_SCALE = (R1+R2)/(R2)  #Voltage Divider Scaling Factor, inverse is scaling factor of voltage divider reading
REPORT_PERIOD_SECOND = 1 #Updates output every second
M = 0.995 #Linear regression slope
B = 0.166071 #Linear regression y intercept
PICO_OFFSET = -0.07

#Initializing ADC
adc = ADC(ADC_PIN)

while True:
    v_adc =  float(adc.read_u16() / ADC_MAX) * V_REF # calculate adc voltage
    v_bat = v_adc * DIVIDER_SCALE# upscale divided voltage to original value
    v_corrected = (v_bat - B) / M + PICO_OFFSET#Corrected reading based on linear regression

    print("{:.2f}".format(v_corrected)) #Output result via USB (serial)
    sleep(REPORT_PERIOD_SECOND) #Wait one second before next measurement
