import time
from torpico_code.usb_sensor_receiver_torpico import USB_Sensor_Receiver_Torpico

sensor = USB_Sensor_Receiver_Torpico()

while True:
    data = sensor.read_sensors()

    if data is None:
        print("Sensor data cannot be read")
    else:
        print(data)

    time.sleep(1) # Delay in seconds