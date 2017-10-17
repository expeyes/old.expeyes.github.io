from pylab import *

tmax = 5
dt = .5
t = arange(0, tmax, dt)     # time array
pos = 2 * t**2          	# simulate experimental data for x(t)
plot(t, pos, 'o-')			# mark points as circles

tm = arange(dt/2, tmax-dt, dt)	# array of midpoints of t
size = len(tm)

vel = []	# empty list to store velocity, dy/dt
for k in range(size):
	vel.append( (pos[k+1]-pos[k])/dt)  # calculate delta x/ delta y
plot(tm, vel, 'x-')  	# mark points as crosses

accn = gradient(vel)  	# use the numpy function to get gradient
plot(tm, accn, 's-')	# mark points as squares
show()

