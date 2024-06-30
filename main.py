import time
import os
# custom function
import arduSerial
import mouse

# parameter : Arduino
PERIOD = 10 # ms
BAUDRATE = 9600
PORT = "COM8"
TRESHOLD = 0.1

# parameter : mouse
R = 40

# initalize
imu = arduSerial.IMU(PORT, BAUDRATE, PERIOD)
mouse = mouse.mouse(R)

# main
def main():
    while True:
        time.sleep(imu.dt/1000)
        
        # Get data
        droll, dpitch, dyaw = imu.getRPY()
        # Thresholding
        droll, dpitch, dyaw = [0 if -TRESHOLD < a < TRESHOLD else a for a in [droll, dpitch, dyaw]]
        # Move to current + dx, dy
        dx, dy = mouse.move(dyaw, dpitch, PERIOD/1000)
        
        os.system("cls")
        print(f"dx: {dx}\ndy: {dy}")

if __name__ == '__main__':
    main()