# Two dimensional motion, trajectory calculation using scipy.integrate
from pylab import *
from scipy import integrate

ax = 0.0        # Horizontal acceleration
ay = -9.8		# Vertical acceleration

def deriv(PV, t0):		# PV[0] is x, PV[1] is y; PV[2] is Vx, PV[3] is Vy
	return [ PV[2], PV[3], ax, ay ]   # ax = dVx/dt, ay = dVy/dt

dt = .1
t = arange(0, 4, dt) # time array
pv0 = [0,0, 20, 20]  # x, y, Vx and Vy,  at t = 0
pv = integrate.odeint(deriv, pv0, t)	# integrate for position and velocity

plot(pv[:,0], pv[:,1])
show()
