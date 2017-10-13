# Demonstrates differentiation using the definition of dy/dx

from pylab import *

dx = .01
x = arange(0, 10., dx)	  # array of time, independent variable
y = sin(x)                # sine function
plot(x,y)

xx = arange(dx/2, 10.-dx, dx)  # array to mark midpoints of array x
size = len(xx)
dydx = []

for k in range(size):
	dydx.append( (y[k+1]-y[k])/dx)  # calculate delta x / delta y
plot(xx, dydx)
show()


