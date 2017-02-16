# Numerical integration to calculate value of pi
from pylab import *

r = 1.0					# radius of circle
dx = .05				# integration delta x
x = arange(0, r+dx, dx)	# array ox x-coordinates
y = sqrt(r**2 - x**2)   # equation of circle, r^2 = x^2 + y^2
plot(x,y, 'x-')
show()

N = len(y)
A = 0.0
for k in range(N-1):			# N-1 trapezoids 
	at = (y[k] + y[k+1])* dx/2	# area of one trapezoid 
	A += at						# add up the areas

print A*4, pi		# compare area of circle calculated and pi*r^2

