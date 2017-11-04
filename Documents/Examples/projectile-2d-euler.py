# Trajectory of a projectile in xy plane, using Euler's method
from pylab import *

t = 0.0         # Stating time and stepsize used
dt = 0.1       	
x = 0.0         # initial x and y positions
y = 0.0
vx = 20.0       # initial velocity
vy = 20.0
ay = -9.8		# acceleration in y direction, gravity

tm = arange(0, 4, dt)	# array of time
N = len(tm)
xa = zeros(N)	# arrays to store computed values of x and y
ya = zeros(N)

for k in range(N):
  xa[k] = x			# add x to the x array
  ya[k] = y
  vy = vy + ay * dt		# compute velocity after dt, using acceleration
  x = x + vx * dt 		# update x using dx = v * dt
  y = y + vy * dt              

plot(xa,ya)
xlabel('x')
show()
