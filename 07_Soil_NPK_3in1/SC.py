import minimalmodbus
import serial
import time

PORT = '/dev/ttyUSB0'
SLAVE_ID = 1
BAUD_RATE = 9600

def read_soil_data():
    try:
        instrument = minimalmodbus.Instrument(PORT, SLAVE_ID)
        instrument.serial.baudrate = BAUD_RATE
        instrument.serial.timeout = 1
        instrument.mode = minimalmodbus.MODE_RTU
        
        
        data = instrument.read_registers(0, 3)
        
        if data:
            n = data[0]
            p = data[1]
            k = data[2]
            
            print(f"N: {n}")
            print(f"P: {p}")
            print(f"K: {k}")
            
    except Exception as e:
        print(f"eror: {e}")

if __name__ == "__main__":
    while True:
        read_soil_data()
        print("-" * 30)
        time.sleep(2)