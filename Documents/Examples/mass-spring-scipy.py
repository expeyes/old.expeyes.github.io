# Mass on Spring system equation, solved using scipy.integrate.odeint()
from pylab import *
from scipy import integrate

k = 10.0
m = 1.0

def diff_eqn(pv, t0):				# F = -k*x  ; a = dv/dt = F/m
	return [pv[1], -k/m * pv[0] ]

pv0 = [2,0]					# displacement & velocity at t = 0
t = np.arange(0, 10, 0.01)    			# time span and steps
pv = integrate.odeint(diff_eqn, pv0, t)	# integrate
  
plot(t, pv[:,0])	# Extract first column from the 2d array
plot(t, pv[:,1])
show()  
