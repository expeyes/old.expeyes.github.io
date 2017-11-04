from pylab import *
from scipy import integrate

R = 0.0         # resistor
L = 1.0         # inductor
C = 1.0 	    # capacior

iv0 = [5.0, 0.0]  #initial values of i and v

def solver(iv, t):  # iv[0] is i(t), iv[1] is v(t)
	iprime = -iv[1]/L - R/L*iv[0]
	vprime = iv[0]/C
	return [iprime, vprime]	 	

tmax =10.0     				# simulate from t=0 to tmax seconds
step = tmax / 10000 		# do the integration in 10000 steps
t = arange(0, tmax, step)	# time array for the integrator
nt = integrate.odeint(solver, iv0, t)   # integrate, refer scipy docs

i = nt[:,0]  # extract the current from the resulting array
v = nt[:,1]  # extract the voltage from the resulting array

Le = 0.5 * L * i**2    # Inductive energy as a function of time
Ce = 0.5 * C * v**2	   # Capacitive energy as a function of time

plot(t,v, label='V')
plot(t,i, label='I')
plot(t, Le, label='El')
plot(t, Ce, label='Ec')
plot(t, Ce+Le,label='Etot')
legend(framealpha=0.5)

show()

