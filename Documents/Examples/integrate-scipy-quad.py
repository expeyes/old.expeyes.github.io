# Numerical integration to calculate value of pi

from numpy import *
from scipy import integrate

r = 1.0					# radius of the circle
def f(x):				
	return sqrt(r**2 - x**2)	# equation of circle

res = integrate.quad(f, 0, 1)  # integrate from 0 to r

print res[0] * 4, pi    #compare area x 4 to area of unit circle
