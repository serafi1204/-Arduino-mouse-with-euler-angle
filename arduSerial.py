# Serial communication
import serial
import os
import time

class IMU:
    def __init__(self, port='COM8', baudrate=9600, dt=10): 
        self.dt = dt
        self.seri = None
        self.port = port
        self.baudrate = baudrate
        self.line = ""
        
        self.connect()
        
    def connect(self):
        success = 0
        cnt = 0
        while (success == 0):
            try:
                self.seri = serial.Serial(port=self.port, baudrate=self.baudrate,
                                parity=serial.PARITY_NONE,
                                stopbits=serial.STOPBITS_ONE,
                                bytesize=serial.EIGHTBITS) 
                
                success = 1
                
                os.system('cls')
                print(f"Succes to Connect!")
                
                
            except serial.serialutil.SerialException:
                cnt += 1
                time.sleep(1)
                
                os.system('cls')
                print(f"Fail to Connect. Retry....{cnt}")

    def getLine(self):
        cnt = 0
        line = ""
        word = ""
        
        self.seri.flushInput()
        while True:
            word = self.seri.read().decode('ascii')
            
            if (cnt == 1) : line += word
            if (word == "\n") : cnt += 1
            if (cnt == 2) : break
            

        return line
        
    def getRPY(self):
        success = 0
        cnt = 0
        while (success == 0):
            try :
                self.line = self.getLine()
                roll, pitch, yaw = list(map(float, self.line.split(" ")))
                
                success = 1
                
                if (cnt != 0):
                    os.system('cls')
                    print(f"Succes to communication!")
                
            except ValueError:
                cnt += 1
                
                os.system('cls')
                print(f"Communication error.(line: {self.line}) Retry....{cnt}")
                
            except serial.serialutil.SerialException:
                self.connect()
                
        return roll, pitch, yaw
        
