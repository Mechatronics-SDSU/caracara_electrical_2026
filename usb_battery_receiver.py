import time
import serial

baud_rate = 115200


class USB_Battery_Receiver:
    def __init__(self):
        self.srl = None
        self.port = None
        self.ports = ["/dev/ttyACM0", "/dev/ttyACM1"]

        self.connect()

    def connect(self):
        if self.srl is not None:
            return True

        for port in self.ports:
            try:
                self.srl = serial.Serial(port, baud_rate, timeout=1)
                self.port = port
                print(f"Connected to battery monitor on {port}")
                time.sleep(2)
                self.srl.reset_input_buffer()
                return True

            except serial.SerialException as e:
                print(f"Failed to connect on {port}: {e}")

        return False

    def disconnect(self):
        if self.srl is not None:
            try:
                self.srl.close()
            except serial.SerialException:
                pass

        print("Battery monitor disconnected")
        self.srl = None
        self.port = None

    def read_voltage(self):
        if self.srl is None:
            self.connect()
            return None

        try:
            line = self.srl.readline().decode(errors="ignore").strip()

            if line:
                return float(line)

        except (serial.SerialException, OSError):
            self.disconnect()
            return None

        except ValueError:
            return None

        return None
