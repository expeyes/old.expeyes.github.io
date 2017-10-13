from pylab import *

m = 2.0		# Mass of the particle
q = 5.0		# Charge
t = 0.0
dt = 0.001
N = 5000	# number of integrating steps

E = array([0.0, 0.0, 0.0])
B = array([0.0, 0.0, 5.0])

pos = zeros([N,3])   # array of N positions
pos[0] = [5,0,0]     # inital position. x, y and z
vel = zeros([N,3])
vel[0] = [20,0,0]    # initial velocity. vx, vy and vz

for k in range(N-1):
	F = q * (E + cross(vel[k],B)) 		# F = q * [E (v x B)]
	vel[k+1] = vel[k] + F/m * dt		# a = F/m; dv = a.dt
	pos[k+1] = pos[k] + vel[k] * dt		# dx = v.dt
	t = t + dt

from mpl_toolkits.mplot3d import Axes3D
ax = Axes3D(figure())
ax.plot(pos[:,0], pos[:,1], pos[:,2])		# 3d plot of x, y and z
ax.set_zlabel('Z axis')
show()
