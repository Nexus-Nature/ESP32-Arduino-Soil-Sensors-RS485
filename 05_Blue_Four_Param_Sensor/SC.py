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
        
        
        data = instrument.read_registers(0, 4)
        
        if data:
            mois = data[0] / 10.0
            temp = data[1] / 10.0
            ec = data[2]
            ph = data[3] / 10.0
            print(f"SoilMois: {mois}%")
            print(f"SoilTemp: {temp}°C")
            print(f"EC: {ec} us/cm")
            print(f"pH: {ph}")
            
    except Exception as e:
        print(f"eror: {e}")

if __name__ == "__main__":
    while True:
        read_soil_data()
        print("-" * 30)
        time.sleep(2)