# Radioactive decay plot, using Euler method of solving diff. equation
from pylab import *

t = 0.
dt = 0.1
L = .5			# decay constant	
N = 10000.0		# Number of atoms at t = 0

ta = [t]	# list for storing time
na = [N]    # and instantaneous number of atoms

while t < 5:
	dn = -L * N * dt
	N = N + dn 
	t = t + dt
	ta.append(t)
	na.append(N)

plot(ta,na)
show()

