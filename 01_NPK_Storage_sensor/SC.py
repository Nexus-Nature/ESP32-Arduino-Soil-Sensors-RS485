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
        

        data = instrument.read_registers(0, 7)
        
        if data:
            Mois = data[0] / 10.0
            temp = data[1] / 10.0
            ec = data[2]
            ph = data[3] / 10.0
            n = data[4]
            p = data[5]
            k = data[6]
            
            print(f"SoilMois: {Mois}%")
            print(f"SoilTemp: {temp}°C")
            print(f"EC: {ec} us/cm")
            print(f"pH: {ph}")
            print(f"N: {n} mg/kg")
            print(f"P: {p} mg/kg")
            print(f"K: {k} mg/kg")
            
    except Exception as e:
        print(f"Eror: {e}")

if __name__ == "__main__":
    while True:
        read_soil_data()
        print("-" * 30)
        time.sleep(2)
