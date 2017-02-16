from pylab import *
from scipy import integrate


R = 0.0    # resistor
L = 1.0     # inductor
C = 1.0		# capacior
omega0 = 1./sqrt(L*C)

Vap = 1.0   		# driving sine wave amplitude
Domega = 3*omega0	# driving angular frequency

iv0 = [5.0, 0.0]  #initial values of i and v

def solver(iv, t):   # iv[0] is i(t), iv[1] is v(t)	
	iprime = Vap*sin(Domega*t)/L -iv[1]/L - R/L*iv[0]
	vprime = iv[0]/C
	return [iprime, vprime]	
	
tmax = 30*pi     				# simulate from t=0 to tmax seconds
step = tmax / 10000 		# do the integration in 10000 steps
t = arange(0, tmax, step)	# time array for the integrator
 
nt = integrate.odeint(solver, iv0, t)   # integrate, refer scipy docs
i = nt[:,0]  # extract the current from the resulting array
v = nt[:,1]  # extract the voltage from the resulting array

plot(t,v, label='V')
plot(t,i, label='I')
plot(t, Vap*sin(Domega*t), label ='Va')
legend(framealpha=0.5)
show()

#for k in range(10000): 	print t[k], v[k]


