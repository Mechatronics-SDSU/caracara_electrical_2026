import time
from usb_sensor_receiver import USB_Sensor_Receiver

receiver = USB_Sensor_Receiver()

while True:
    data = receiver.read_sensors()
    if data is not None:
        print(data)