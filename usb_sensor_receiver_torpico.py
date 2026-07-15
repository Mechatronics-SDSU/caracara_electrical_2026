import time
import serial # Allows communication over USB
import serial.tools.list_ports # Lists every serial port connected to a computer

BAUD_RATE = 115200 # Serial communication speed, baud is unit for symbol rate or modulation rate in symbols or pules per second
DEVICE_ID = "SENSOR" # Handshake string (checks if it is the correct device)
WAIT_TIME_SECONDS = 2

class USB_Sensor_Receiver_Torpico:
    def __init__(self):
        self.srl = None # Initially no serial connection, stores empty serial object

    def find_serial_ports(self):
            ports = [] # List that will be filled with serial ports connected to a computer

            for port in serial.tools.list_ports.comports(): # Loops through all comports
                if port.device != "/dev/ttyACM0" or port.device != "COM4":  # Ignore bad Linux and Windows port
                    ports.append(port.device) # Add useable comports to ports array

            return ports # Return all useable comports

    def connect(self):
        if self.srl is not None: # If connected (port exists), then don't reconnect
            return True

        ports = self.find_serial_ports() # Calls above function to find all available comports

        for port in ports: # Loop through all available ports
            try:
                test_srl = serial.Serial(port, BAUD_RATE, timeout=1) # Opens port, if successful then becomes connection
                print(f"Testing port {port}...") # Prints test port

                time.sleep(WAIT_TIME_SECONDS) # Waits for Pico to reboot
                test_srl.reset_input_buffer() # Deletes old serial data

                for _ in range(5): # Read 5 lines, _ is placeholder variable
                    line = test_srl.readline().decode(errors="ignore").strip() # Reads one line from USB, converts bytes to text, removes formatting
                    print(f"Handshake line: {repr(line)}") # Shows what was received from Pico

                    if line.startswith(DEVICE_ID): # Check if line begins with SENSOR
                        self.srl = test_srl # Stores serial connection 
                        self.port = port # Stores communication port
                        print(f"Connected on {port}") # Displays communication port
                        return True # Connection was found, don't search

                test_srl.close() # Close if not correct Pico (handshake didn't occur)

            except serial.SerialException as e: #Handles connection errors, doesn't crash bcz of them
                print(f"Failed to connect on {port}: {e}")

        return False #No ports found

    def disconnect(self):
        if self.srl is not None: #If communication port exists
            try:
                self.srl.close() # Close current port
            except serial.SerialException: # Ignore errors
                pass

        print("Sensor monitor disconnected")
        self.srl = None # Receiver disconnected
        self.port = None # Communcation port disconnected, no active port

    def read_sensors(self):
        if self.srl is None: #If no connection
            self.connect() # Try reconnecting
            return None # No data returned since no connection

        try:
            line = self.srl.readline().decode(errors="ignore").strip() # Reads one line from USB, converts bytes to text, removes formatting

            if not line: # If no data
                return None # Return no data since nothing to send

            values = {} # Dictionary to hold values from Pico and depth sensor readings

            for item in line.split(","): #Key values pair split via :
                key, value = item.split(":")
                values[key] = float(value)

            return { # Returns dictionary, key-value pair
                "battery_voltage": values.get("BAT"),
                "depth_m": values.get("DEPTH"),
                "pressure_mbar": values.get("PRESS"),
                "temperature_c": values.get("TEMP"),
            }

        except (serial.SerialException, OSError): # Pico disconnects/unplugged, communication port disappears
            self.disconnect() # Close port
            return None # No data receiver

        except (ValueError, KeyError): # Handles unexpected values
            return None # No proper data received