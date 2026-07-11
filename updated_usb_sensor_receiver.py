import time
import serial

baud_rate = 115200


class Updated_USB_Sensor_Receiver:
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
                print(f"Connected to sensor monitor on {port}")

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

        print("Sensor monitor disconnected")
        self.srl = None
        self.port = None

    def read_sensors(self):
        if self.srl is None:
            self.connect()
            return None

        try:
            line = self.srl.readline().decode(errors="ignore").strip()

            if not line:
                return None

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

        except (serial.SerialException, OSError):
            self.disconnect()
            return None

        except (ValueError, KeyError):
            return None