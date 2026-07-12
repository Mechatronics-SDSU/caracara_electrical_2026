import time
from usb_battery_receiver_caracara import USB_Battery_Receiver_Caracara

battery = USB_Battery_Receiver_Caracara()

while True:
    voltage = battery.read_voltage()

    if voltage is not None:
        print(f"Battery voltage: {voltage:.2f} V")
    
    #Delay in seconds
    time.sleep(1)