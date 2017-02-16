from pylab import *
from scipy import integrate

L = 5  # decay constant

def derivative(y, t0):  	
	return -L * y		# dN/dt = -L * N , radioactive decay

N = 1000				# value at t = 0
t = arange(0, 1, 0.01)	    				# time span and steps

nt = integrate.odeint(derivative, N, t)		# integrate, refer scipy docs
print nt

plot(t, nt[:,0])  # extract the first column from the 2D array
show()

