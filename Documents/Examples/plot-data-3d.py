#Reads multi-column text data from files and plots the first 3 columns

from pylab import *
from mpl_toolkits.mplot3d import Axes3D
ax = Axes3D(figure())

dat = loadtxt('xyz.txt', unpack=True)	# reads multicolumn data

ax.plot(dat[0],dat[1],dat[2])
ax.set_xlabel('X axis')
ax.set_ylabel('Y axis')
ax.set_zlabel('Z axis')
show()
