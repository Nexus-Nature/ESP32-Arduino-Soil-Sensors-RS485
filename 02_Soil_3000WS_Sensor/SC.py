import minimalmodbus
import serial
import time

PORT = '/dev/ttyUSB0'
SLAVE_ID = 1
BAUD_RATE = 4800

def read_soil_data():
    try:
        instrument = minimalmodbus.Instrument(PORT, SLAVE_ID)
        instrument.serial.baudrate = BAUD_RATE
        instrument.serial.timeout = 1
        instrument.mode = minimalmodbus.MODE_RTU
        
        
        data = instrument.read_registers(0, 2)
        
        if data:
            Mois = data[0] / 10.0
            SoilTemp = data[1] / 10.0
            
            print(f"SoilMois: {Mois}%")
            print(f"SoilTemp: {SoilTemp}°C")
            
    except Exception as e:
        print(f"eror: {e}")

if __name__ == "__main__":
    while True:
        read_soil_data()
        print("-" * 30)
        time.sleep(2)
