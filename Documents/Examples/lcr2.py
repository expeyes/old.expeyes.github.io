from pylab import *
from scipy import integrate
from scipy.optimize import leastsq

def sine_erf(p,y,x):					
  return y - p[0] * sin(2*pi*p[1]*x+p[2])+p[3]

def sinefit(ya, xa):
  par = [5.0, 1000, 0.0, 0.0] # Amp, freq, phase , offset
  plsq = leastsq(sine_erf, par,args=(ya,xa))
  print plsq

R = 0.0         # resistor
L = 1.0e-3      # inductor
C = 1.0e-3	# capacior

iv0 = [0.0, 5.0]  #initial values of i and v

def solver(IV, t0):  	
	return [ -IV[1]/L - R/L*IV[0], IV[0]/C]	

tmax = .1     # seconds
step = tmax / 10000
t = arange(0, tmax, step)	# time span and steps

nt = integrate.odeint(solver, iv0, t)   # integrate, refer scipy docs


plot(t, nt[:,0])  # extract the first column from the 2D array
plot(t, nt[:,1])  # extract the second column from the 2D array
sinefit(t, nt[:,0])
show()

