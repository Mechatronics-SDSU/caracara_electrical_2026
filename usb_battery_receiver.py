import serial

baud_rate = 115200


class USB_Battery_Receiver:
    def __init__(self):
        self.srl = None

        for port in ["/dev/ttyACM0", "/dev/ttyACM1"]:
            try:
                self.srl = serial.Serial(port, baud_rate, timeout=1)
                print(f"Connected to battery monitor on {port}")
                break
            except serial.SerialException as e:
                print(f"Failed to connect on {port}: {e}")

    def read_voltage(self):
        if self.srl is None:
            return None

        try:
            line = self.srl.readline().decode().strip()

            if line:
                return float(line)

        except ValueError:
            return None

        return None