import serial, time, csv

COM_PORT = 'COM6'  # Nhớ sửa lại cổng COM
BAUD_RATE = 115200

# 5 Trạng thái (Nhãn) để điều khiển xe
scenarios = [
    {"label": 0, "msg": "NHAN 0: DE YEN MACH TREN BAN (DUNG XE)"},
    {"label": 1, "msg": "NHAN 1: NGHIENG TOI TRUOC (CHAY TOI)"},
    {"label": 2, "msg": "NHAN 2: NGHIENG RA SAU (CHAY LUI)"},
    {"label": 3, "msg": "NHAN 3: NGHIENG SANG PHAI (RE PHAI)"},
    {"label": 4, "msg": "NHAN 4: NGHIENG SANG TRAI (RE TRAI)"}
]

ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

with open('car_dataset.csv', 'w', newline='') as f:
    csv.writer(f).writerow(['pitch', 'roll', 'label'])

for sc in scenarios:
    print(f"\n{sc['msg']}")
    print(">> Chuan bi tu the tay... Ghi sau 3 giay...")
    time.sleep(3)

    print(">> DANG GHI DU LIEU (5 giay)...")
    ser.reset_input_buffer()
    start_time = time.time()
    rows = []

    while time.time() - start_time < 5:
        if ser.in_waiting:
            try:
                line = ser.readline().decode().strip()
                p, r = map(int, line.split(','))
                rows.append([p, r, sc['label']])
                print(f"p: {p}, r: {r}")
            except:
                pass

    with open('car_dataset.csv', 'a', newline='') as f:
        csv.writer(f).writerows(rows)

ser.close()
print("\nHOAN THANH THU THAP DU LIEU!")