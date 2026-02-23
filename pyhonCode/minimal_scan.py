import minimalmodbus
import serial
import time

# تأكد من المنفذ (USB0 أو USB1 حسب ما ظهر معك)
PORT = '/dev/ttyUSB1' 

def scan():
    print(f"بدء المسح على {PORT} بسرعة 4800...")
    
    # حلقة تجربة الـ IDs من 1 إلى 247
    for unit_id in range(1, 248):
        try:
            instrument = minimalmodbus.Instrument(PORT, unit_id)
            instrument.serial.baudrate = 4800
            instrument.serial.timeout = 0.2
            
            # محاولة قراءة سجل واحد (غالباً العنوان 0)
            # إذا استجاب السنسور، فهذا يعني أننا وجدنا الـ ID
            value = instrument.read_register(0, 0)
            print(f"\n[+] مبروك! وجدنا جهاز على ID: {unit_id}")
            print(f"القيمة المقروءة من العنوان 0 هي: {value}")
            return # توقف عند العثور على أول جهاز
        except (serial.SerialException, minimalmodbus.NoResponseError):
            print(f"تجربة ID {unit_id}...", end='\r')
        except Exception as e:
            print(f"\nخطأ غير متوقع عند ID {unit_id}: {e}")
            
    print("\nانتهى المسح ولم نجد أي استجابة.")

if __name__ == "__main__":
    scan()
