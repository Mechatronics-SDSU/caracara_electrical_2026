from usb_battery_receiver import USB_Battery_Receiver

battery = USB_Battery_Receiver()

while True:
    voltage = battery.read_voltage()

    if voltage is not None:
        print(f"Battery voltage: {voltage:.2f} V")