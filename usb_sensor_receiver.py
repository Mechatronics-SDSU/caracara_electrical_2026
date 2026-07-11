import serial

baud_rate = 115200


class USB_Sensor_Receiver:
    def __init__(self):
        self.srl = None

        for port in ["/dev/ttyACM0", "/dev/ttyACM1"]:
            try:
                self.srl = serial.Serial(port, baud_rate, timeout=1)
                print(f"Connected to battery monitor on {port}")
                break
            except serial.SerialException as e:
                print(f"Failed to connect on {port}: {e}")

    def read_sensors(self):
        if self.srl is None:
            return None

        try:
            line = self.srl.readline().decode(errors="ignore").strip()

            if not line:
                return None

            # Expected:
            # BAT:12.34,DEPTH:0.523,PRESS:1054.22,TEMP:25.91
            values = {}

            for item in line.split(","):
                key, value = item.split(":")
                values[key] = float(value)

            return {
                "battery_voltage": values.get("BAT"),
                "depth_m": values.get("DEPTH"),
                "pressure_mbar": values.get("PRESS"),
                "temperature_c": values.get("TEMP"),
            }

        except (ValueError, KeyError):
            return None