# Charged particle trajectory under electric and magnetic fields
from pylab import *
from scipy import integrate

m = 25.0	# Mass and 
q = 5.0		# Charge of the particle
Ex = 0.0	# Electric Field vector
Ey = 0.0
Ez = 0.1
Bx = 0.0	# Magnetic field vector
By = 0.0
Bz = 5.0

def solver(X, t0): # X contains x,y,z and dx,dy,dz , 6 elements
	vx = X[3]
	vy = X[4]
	vz = X[5]
	ax = q * (Ex + (vy * Bz) - (vz * By) ) /m	# Lorentz force / mass
	ay = q * (Ey - (vx * Bz) + (vz * Bx) ) /m
	az = q * (Ez + (vx * By) - (vy * Bx) ) /m
	return [vx, vy, vz, ax, ay, az ]
	
pv0 = [0,0,0, 0,1,0]		# position & velocity at t = 0
t = arange(0, 50, 0.01)		# duration and steps
pv = integrate.odeint(solver, pv0, t)		# integrate

from mpl_toolkits.mplot3d import Axes3D
ax = Axes3D(figure())
ax.plot(pv[:,0], pv[:,1], pv[:,2])			# 3d plot of x, y and z
ax.set_zlabel('Z axis')
show()


