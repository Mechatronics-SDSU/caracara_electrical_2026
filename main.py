from machine import ADC, Pin, I2C
from utime import sleep, sleep_ms

# -------------------------------------------------
# Battery ADC settings
# -------------------------------------------------
ADC_PIN = 26
ADC_MAX = 65535.0
V_REF = 3.283

R1 = 100000.0
R2 = 22000.0
DIVIDER_SCALE = (R1 + R2) / R2

M = 0.995
B = 0.166071
PICO_OFFSET = -0.07

adc = ADC(ADC_PIN)

def read_battery_voltage():
    v_adc = float(adc.read_u16() / ADC_MAX) * V_REF
    v_bat = v_adc * DIVIDER_SCALE
    v_corrected = (v_bat - B) / M + PICO_OFFSET
    return v_corrected


# -------------------------------------------------
# Bar02 / MS5837 settings
# -------------------------------------------------
MS5837_ADDR = 0x76

CMD_RESET = 0x1E
CMD_ADC_READ = 0x00
CMD_PROM_READ = 0xA0
CMD_CONVERT_D1_8192 = 0x4A  # Pressure
CMD_CONVERT_D2_8192 = 0x5A  # Temperature

FLUID_DENSITY = 997.0       # Freshwater kg/m^3
GRAVITY = 9.80665

i2c = I2C(1, sda=Pin(18), scl=Pin(19), freq=50000)

C = [0] * 8
pressure_mbar = 0.0
temperature_c = 0.0
surface_pressure_mbar = 1013.25


def bar02_init():
    global C

    print("Resetting Bar02...")
    i2c.writeto(MS5837_ADDR, bytes([CMD_RESET]))
    sleep_ms(500)

    print("After reset scan:", i2c.scan())

    for i in range(7):  # Read C0 through C6 only
        cmd = CMD_PROM_READ + i * 2
        i2c.writeto(MS5837_ADDR, bytes([cmd]))
        sleep_ms(20)
        data = i2c.readfrom(MS5837_ADDR, 2)
        C[i] = (data[0] << 8) | data[1]

    print("Calibration coefficients:", C)


def bar02_read_adc():
    i2c.writeto(MS5837_ADDR, bytes([CMD_ADC_READ]))
    data = i2c.readfrom(MS5837_ADDR, 3)
    return (data[0] << 16) | (data[1] << 8) | data[2]

def bar02_read():
    global pressure_mbar, temperature_c

    i2c.writeto(MS5837_ADDR, bytes([CMD_CONVERT_D1_8192]))
    sleep_ms(20)
    D1 = bar02_read_adc()

    i2c.writeto(MS5837_ADDR, bytes([CMD_CONVERT_D2_8192]))
    sleep_ms(20)
    D2 = bar02_read_adc()

    dT = D2 - C[5] * 256
    TEMP = 2000 + dT * C[6] / 8388608

    # MS5837-02BA / Bar02 compensation
    SENS = C[1] * 65536 + (C[3] * dT) / 128
    OFF = C[2] * 131072 + (C[4] * dT) / 64
    P = (D1 * SENS / 2097152 - OFF) / 32768

    temperature_c = TEMP / 100.0
    pressure_mbar = P / 100.0


def set_surface_pressure():
    global surface_pressure_mbar
    bar02_read()
    surface_pressure_mbar = pressure_mbar


def get_depth_m():
    pressure_pa = (pressure_mbar - surface_pressure_mbar) * 100.0
    return pressure_pa / (FLUID_DENSITY * GRAVITY)


# -------------------------------------------------
# Main program
# -------------------------------------------------
print("Scanning I2C...")
print(i2c.scan())

bar02_init()

print("Calibrating surface pressure...")
set_surface_pressure()
print("Surface pressure: {:.2f} mbar".format(surface_pressure_mbar))

depth_filtered = 0.0
alpha = 0.15

while True:
    bar02_read()

    battery_v = read_battery_voltage()
    depth_raw = get_depth_m()
    depth_filtered = (1 - alpha) * depth_filtered + alpha * depth_raw

    print(
        "Battery: {:.2f} V | Pressure: {:.2f} mbar | Temp: {:.2f} C | Depth: {:.3f} m".format(
            battery_v,
            pressure_mbar,
            temperature_c,
            depth_filtered
        )
    )

    sleep(1)