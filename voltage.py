#Importing modules: machine (class that has functions for hardware), ADC (analog to digital converter), time (for waiting + delays)
from machine import ADC
import time

#----------------------------------------------
# Values for Pico Hardware
#----------------------------------------------

ADC_PIN = 26    #Connection to GPIO26 (ADC0 on Pico)
ADC_MAX = 65535.0 #MircoPython converts 12 bits to 16 bits, .0 for float values
V_REF = 3.3     #[Volts], Pico IO voltage is 3.3V 

#----------------------------------------------
# Values for Voltage divider
#----------------------------------------------

R1 = 10000.0   #[Ohms], Top resistor = 100k Ohms
R2 = 22000.0   #[Ohms], Bottom resistor = 22k Ohms

#Voltage Divider Scaling Factor
DIVIDER_SCALE = (R1+R2)/(R2)    #Inverse is scaling factor of voltage divider reading
                                

#----------------------------------------------
# Sampling Values
#----------------------------------------------

NUM_SAMPLES = 16    #Samples to average, 16 chosen since power of 2
SAMPLE_DELAY_MS = 1 #Delay in milliseconds per sample, comes from time constant of 2.7 ms
REPORT_PERIOD_MS = 500 #Updates output every half-second, 500ms

#Initializing ADC
adc = ADC(ADC_PIN)


def read_adc_avg(num_samples = NUM_SAMPLES):
    """
    Takes multiple ADC readings, averaged
    Reduces effects of noise, returns a more stable reading
    """
    total = 0

    for _ in range(num_samples):
        total = total + adc.read_u16()
        time.sleep_ms(SAMPLE_DELAY_MS)
    
    return total / num_samples

def adc_to_voltage(adc_code):
    """
    Converts raw ADC value (0-65535) to voltage at ADC pin
    """
    return(adc_code / ADC_MAX) * V_REF #Ratio of input to max times reference

def scaled_back_voltage(v_adc):
    """
    Converts ADC pin voltage to actual battery voltage
    using the same voltage divider scaling factor
    """

while True:
    #Read and average ADC samples
    avg_code = read_adc_avg()

    #Convert to ADC voltage Pico can read
    v_adc = adc_to_voltage(avg_code)

    #Convert back to full battery voltage
    v_bat = scaled_back_voltage(v_adc)

    #Output result via USB (serial)
    print("Battery Voltage: {:.2f} V".format(v_bat))

    #Wait half second before next measurement
    time.sleep_ms(REPORT_PERIOD_MS)