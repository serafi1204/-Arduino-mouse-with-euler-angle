import os
# Mouse control
import pyautogui

class mouse:
    def __init__(self, r = 10, dt = 10):
        self.W, self.H = pyautogui.size()
        self.r = r
        self.dt = dt
        self.gain = r*dt
        self.x, self.y = pyautogui.position()
        
    def limit(self, a, b, c):
        if (c < a): c = a
        elif (a >= b): c = b
        
        return c
    
    def move(self, dx, dy, t = 0):
        self.x, self.y = pyautogui.position()
        
        dx *= self.gain
        dy *= self.gain
        
        self.x += int(dx)
        self.y += int(dy)
        
        self.x = self.limit(1, self.W, self.x)
        self.y = self.limit(1, self.H, self.y)
        
        pyautogui.moveTo(self.x, self.y, t)
        
        return dx, dy