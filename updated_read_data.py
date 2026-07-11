import time
from updated_usb_sensor_receiver import Updated_USB_Sensor_Receiver

sensor = Updated_USB_Sensor_Receiver()

while True:
    data = sensor.read_sensors()

    if data is None:
        print("Sensor data cannot be read")
    else:
        print(data)

    time.sleep(1)