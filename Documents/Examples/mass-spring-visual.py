# Animation of mass spring system using Euler method of integration
from visual import *

wall = box (pos=(-.5,0,0), length=1, height=1, width=1, color=color.yellow)
ball = sphere (pos=(4,0,0), radius=1, color=color.red)
spring = helix(pos=(0,0,0), axis=(4,0,0), radius=0.35, color=color.white)

t = 0		# start time
dt = 0.1
x = 2.0		# initial position & velocity
v = 0.0
k = 30.0
m = 1.0
damping = .1  # damping factor, change this and run again

while 1:
  rate(20)
  f = -k * x  - damping * v  # velocity dependent damping
  v =  v + (f/m) * dt	  	 # a = F/m;  a = dv/dt
  x =  x + v * dt            # v = dx/dt
  t = t + dt
  spring.length =  4 + x	 # x is the compression or expansion
  ball.x =  x + 4

