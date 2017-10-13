from pylab import *

t = 0.
dt = .1
x = 0
v = 2

ta = []		# list for storing time
xa = []    # and instantaneous number of atoms

while t < 5:
	ta.append(t)
	xa.append(x)
	x = x + v * dt
	t = t + dt

plot(ta,xa)
show()

