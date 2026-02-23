import time
import minimalmodbus

PORT = "/dev/ttyUSB0"
SLAVE_ID = 1

PROFILES = [
    {"name": "7-in-1 M0 T1", "baud": 9600, "start": 0, "count": 7, "mois_i": 0, "temp_i": 1, "has_ec": True, "has_ph": True, "has_npk": True},
    {"name": "7-in-1 M0 T1", "baud": 4800, "start": 0, "count": 7, "mois_i": 0, "temp_i": 1, "has_ec": True, "has_ph": True, "has_npk": True},

    {"name": "7-in-1 T0 M1", "baud": 9600, "start": 0, "count": 7, "mois_i": 1, "temp_i": 0, "has_ec": True, "has_ph": True, "has_npk": True},
    {"name": "7-in-1 T0 M1", "baud": 4800, "start": 0, "count": 7, "mois_i": 1, "temp_i": 0, "has_ec": True, "has_ph": True, "has_npk": True},

    {"name": "4-in-1 M0 T1", "baud": 9600, "start": 0, "count": 4, "mois_i": 0, "temp_i": 1, "has_ec": True, "has_ph": True, "has_npk": False},
    {"name": "4-in-1 M0 T1", "baud": 4800, "start": 0, "count": 4, "mois_i": 0, "temp_i": 1, "has_ec": True, "has_ph": True, "has_npk": False},

    {"name": "4-in-1 T0 M1", "baud": 9600, "start": 0, "count": 4, "mois_i": 1, "temp_i": 0, "has_ec": True, "has_ph": True, "has_npk": False},
    {"name": "4-in-1 T0 M1", "baud": 4800, "start": 0, "count": 4, "mois_i": 1, "temp_i": 0, "has_ec": True, "has_ph": True, "has_npk": False},

    {"name": "2-in-1 M0 T1", "baud": 9600, "start": 0, "count": 2, "mois_i": 0, "temp_i": 1, "has_ec": False, "has_ph": False, "has_npk": False},
    {"name": "2-in-1 M0 T1", "baud": 4800, "start": 0, "count": 2, "mois_i": 0, "temp_i": 1, "has_ec": False, "has_ph": False, "has_npk": False},

    {"name": "2-in-1 T0 M1", "baud": 9600, "start": 0, "count": 2, "mois_i": 1, "temp_i": 0, "has_ec": False, "has_ph": False, "has_npk": False},
    {"name": "2-in-1 T0 M1", "baud": 4800, "start": 0, "count": 2, "mois_i": 1, "temp_i": 0, "has_ec": False, "has_ph": False, "has_npk": False},

    {"name": "NPK-only", "baud": 9600, "start": 0, "count": 3, "npk_only": True},
    {"name": "NPK-only", "baud": 4800, "start": 0, "count": 3, "npk_only": True},
]

def make_instrument(baud: int) -> minimalmodbus.Instrument:
    inst = minimalmodbus.Instrument(PORT, SLAVE_ID)
    inst.serial.baudrate = baud
    inst.serial.timeout = 1.0
    inst.mode = minimalmodbus.MODE_RTU
    inst.clear_buffers_before_each_transaction = True
    return inst

def looks_valid_moist_temp(mois: float, temp: float) -> bool:
    if not (0.0 <= mois <= 100.0):
        return False
    if not (-40.0 <= temp <= 85.0):
        return False
    if mois == 0.0 and temp == 0.0:
        return False
    return True

def looks_valid_ph(ph: float) -> bool:
    return 0.0 <= ph <= 14.0

def autodetect():
    last_error = None
    for p in PROFILES:
        try:
            inst = make_instrument(p["baud"])
            regs = inst.read_registers(p["start"], p["count"])
            if not regs:
                continue

            if p.get("npk_only"):
                n, phos, k = regs[0], regs[1], regs[2]
                if (n, phos, k) == (0, 0, 0):
                    continue
                return p, regs

            mois = regs[p["mois_i"]] / 10.0
            temp = regs[p["temp_i"]] / 10.0
            if not looks_valid_moist_temp(mois, temp):
                continue

            if p.get("has_ph"):
                ph = regs[3] / 10.0
                if not looks_valid_ph(ph):
                    continue
            return p, regs

        except Exception as e:
            last_error = e
            continue

    raise RuntimeError(f"Auto-detect failed. Last error: {last_error}")

def decode_and_print(profile, regs):
    print(f"Detected: {profile['name']} | baud={profile['baud']} | regs={profile['count']}")

    if profile.get("npk_only"):
        n, p, k = regs[0], regs[1], regs[2]
        print(f"N: {n} mg/kg")
        print(f"P: {p} mg/kg")
        print(f"K: {k} mg/kg")
        return

    mois = regs[profile["mois_i"]] / 10.0
    temp = regs[profile["temp_i"]] / 10.0
    print(f"SoilMois: {mois}%")
    print(f"SoilTemp: {temp}°C")

    if profile.get("has_ec"):
        ec = regs[2]
        print(f"EC: {ec} us/cm")

    if profile.get("has_ph"):
        ph = regs[3] / 10.0
        print(f"pH: {ph}")

    if profile.get("has_npk"):
        n, p, k = regs[4], regs[5], regs[6]
        print(f"N: {n} mg/kg")
        print(f"P: {p} mg/kg")
        print(f"K: {k} mg/kg")

def main():
    while True:
        try:
            profile, regs = autodetect()
            decode_and_print(profile, regs)
        except Exception as e:
            print(f"Error: {e}")
        print("-" * 30)
        time.sleep(2)

if __name__ == "__main__":
    main()