#Solving the mass spring problem using Euler integration

from pylab import *

k = 10.0	# spring constant
m = 1.0
dt = 0.01
t = 0
x = 2.0
v = 0.0

ta = [t]   # list for time, add start time
xa = [x]   # list for displacement, x = 2 at t= 0
va = [v]   # list for velocity, v=0 at t=0

while t < 5:
  f = -k * x
  v =  v + (f/m) * dt	  # a = F/m;  a = dv/dt
  x =  x + v * dt         # v = dx/dt
  t = t + dt
  ta.append(t)
  xa.append(v)
  va.append(x)
  
plot(ta, xa)
plot(ta, va)
show()  
