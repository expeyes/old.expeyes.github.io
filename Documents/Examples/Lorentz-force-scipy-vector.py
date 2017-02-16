#Trajectory of charged particle in E & M fields, use vector equations
from pylab import *
from scipy import integrate

m = 25.0	# Mass and 
q = 5.0		# Charge of the particle
E = array([0, 0, .1]) # Electric field components Ex,Ey & Ez
B = array([0, 0,  5]) # Magnetic field 

def solver(X, t0): # X is a six element array, t0 required for the solver
	v = array([X[3], X[4], X[5]])  # make the velocity vector
	a = q * (E + cross(v,B)) / m   # F = q v x B ; a = F/m
	return [X[3], X[4], X[5], a[0], a[1], a[2]]	

tmax = 50  # calculate up to 50 seconds			
x0 = [0, 0, 0, 0, 1, 0]					# position & velocity, at t = 0
t = arange(0, tmax, 0.01)	   			# array of time coordinate
pv = integrate.odeint(solver, x0, t)	# integrate for position and velocity

savetxt('xyz.txt',pv)		# saves 6 columns to file, x,y,x, Vx, Vy, Vz

from mpl_toolkits.mplot3d import Axes3D
ax = Axes3D(figure())
ax.plot(pv[:,0], pv[:,1], pv[:,2])			# 3d plot of x, y and z
ax.set_zlabel('Z axis')
show()

