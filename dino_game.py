from serial.tools import list_ports
import serial
import time
import csv
import pyautogui

def milis():
    return int(round(time.time() * 1000))

f = open("test.csv", "w", newline='')
f.truncate()

serialCom = serial.Serial("COM11", 115200, timeout=1)

serialCom.setDTR(False)
time.sleep(1)
serialCom.flushInput()
serialCom.setDTR(True)

timer = milis()
start_time = time.time()  # Record the start time

latency = 24
duration = 60  # Timer duration in seconds (1 minute)

while(True):
    try:
        s_bytes = serialCom.readline()
        decoded_bytes = s_bytes.decode("utf-8").strip("\r\n")
        # print(decoded_bytes)

        values = decoded_bytes.split(",")

        value = float(values[0])
        # print(value)

        if((milis() - timer) > latency):
            timer = milis()
            if (abs(value) >2.5):
                pyautogui.press("space")
                print("jump")

        serialCom.flushInput()

        writer = csv.writer(f)
        writer.writerow(values)  

        # Check if 1 minute has elapsed
        if((time.time() - start_time) > duration):
            print("1 minute elapsed. Stopping the program.")
            break
    except:
        print("error")

f.close()
