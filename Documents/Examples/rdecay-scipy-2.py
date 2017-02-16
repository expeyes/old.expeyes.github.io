# Radio active decay. Solve 2 equations DN/dt = -L * N at a time
from pylab import *
from scipy import integrate

L1 = .5  		# decay constant
N1 = 1000		# value at t = 0
L2 = 1.0
N2 = 2000

def derivative(Y, t0):  	
	return [-L1 * Y[0], -L2 * Y[1] ]	# dN/dt = -L * N , radioactive decay

t = arange(0, 3, 0.1)		# time span and steps
nt0 = [N1, N2]   			# values of Ns at t=0
nt = integrate.odeint(derivative, nt0, t)		# integrate

plot(t, nt[:,0])  # extract the first column from the 2D array
plot(t, nt[:,1])  # extract the first column from the 2D array
show()

