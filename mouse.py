import os
# Mouse control
import pyautogui

pyautogui.FAILSAFE = False

class mouse:
    def __init__(self, r_px = 10, r_nx=10, r_py=10, r_ny=10, dt = 10):
        self.W, self.H = pyautogui.size()
        self.r_px = r_px
        self.r_nx = r_nx
        self.r_py = r_py
        self.r_ny = r_ny
        self.dt = dt
        self.x, self.y = pyautogui.position()
        
    def limit(self, a, b, c):
        if (c < a): c = a
        elif (a >= b): c = b
        
        return c
    
    def move(self, dx, dy, t = 0):
        self.x, self.y = pyautogui.position()
        
        dx *= self.dt * (self.r_px if dx > 0 else self.r_nx) 
        dy *= self.dt * (self.r_py if dy > 0 else self.r_ny)
        
        self.x += int(dx)
        self.y += int(dy)
        
        self.x = self.limit(1, self.H, self.x)
        self.y = self.limit(1, self.W, self.y)
        
        pyautogui.moveTo(self.x, self.y, t)
        
        return dx, dy
