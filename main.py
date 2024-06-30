import time
import os
import numpy as np
# custom function
import py_function.arduSerial as arduSerial
import py_function.mouse as mouse

# parameter
DEBUG = 1
CURV_GAIN = 800

# parameter : Arduino
PERIOD = 1 # ms
BAUDRATE = 38400
PORT = "COM8"
TRESHOLD = 0.005

# parameter : mouse
R_NX = 150
R_PX = R_NX
R_NY = R_NX*1.2
R_PY = R_NY*1.5

# initalize
imu = arduSerial.IMU(PORT, BAUDRATE, PERIOD)
mouse = mouse.mouse(R_PX, R_NX, R_PY, R_NY, PERIOD)

# main
def main():
    while True:
        start = time.time()
        
        # Get data
        droll, dpitch, dyaw = imu.getRPY()
        # Thresholding
        droll_th, dpitch_th, dyaw_th = [0 if -TRESHOLD < a < TRESHOLD else a for a in [droll, dpitch, dyaw]]
        droll_np, dpitch_np, dyaw_np = [a if -0.05< a < 0.05 else a**3*CURV_GAIN for a in [droll_th, dpitch_th, dyaw_th]]
        
        # Update freq
        end = time.time()
        p_s = end-start
        p_ms = p_s*1000
        imu.dt = p_ms
        mouse.dt = p_ms
        
        # Move to current + dx, dy
        dx, dy = mouse.move(dyaw_th, dpitch_th, p_s)

        if (DEBUG == 1):
            os.system("cls")
            print(f"droll: {droll:0.4f}")
            print(f"dpitch: {dpitch:0.4f}")
            print(f"dyaw: {dyaw:0.4f}")
            print(f"dx: {dx}")
            print(f"dy: {dy}")
            print(f"t: {end - start}")
            
            
if __name__ == '__main__':
    main()
