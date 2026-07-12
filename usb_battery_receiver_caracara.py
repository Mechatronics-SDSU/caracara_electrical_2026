import serial
import serial.tools.list_ports

baud_rate = 115200

class USB_Battery_Receiver_Caracara:
    def __init__(self):
        self.srl = None

        #Checks all available ports, connects to first available one
        ports = self.find_serial_ports()

        #For loop, loops through all available ports
        for port in ports:
            try:
                #Connects to available port
                self.srl = serial.Serial(port, baud_rate, timeout=1, writetimeout = 1)
                print(f"Connected to battery monitor on {port}")
                break
            except serial.SerialException as e:
                print(f"Failed to connect on {port}: {e}")


    def find_serial_ports(self):
        ports = []

        for port in serial.tools.list_ports.comports():
            ports.append(port.device)

        return ports

    def read_voltage(self):
        if self.srl is None:
            return None

        try:
            line = self.srl.readline().decode().strip()

            if line:
                return float(line)

        except ValueError:
            return None