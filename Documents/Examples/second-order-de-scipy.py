#Second order diff. equation : d2y/dx2 = -y ; dp/dx = -y ; p = dy/dt

from pylab import *
from scipy import integrate


def derivative(X, t0): 		# X[0] is x, X[1] is dx/dt;
	return [X[1], -X[0] ] 	# derivative of X[0] is X[1] , of X[1] is -y

start = [0,1]					# x and dx/dt at t= 0
t = np.arange(0, 30, 0.01)	  	# start time, stop and stepsize
result = integrate.odeint(derivative, start, t)	# integrate

plot(t, result[:, 0])	# extract first column from 2d array
plot(t, result[:, 1])	# extract second column
show()

