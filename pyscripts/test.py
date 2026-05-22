import serial
import time

robot = serial.Serial('COM3', 115200, timeout=1)

time.sleep(2)

while True:
    user_input = input()
    command = f"{user_input}\n"
    robot.write(command.encode('utf-8'))
    
    print(f"Отправлено: {command.strip()}")
