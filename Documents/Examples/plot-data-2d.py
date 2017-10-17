#Reads multi-column text data from files and plots the first 2 columns

from pylab import *

dat = loadtxt('xyz.txt', unpack=True)	# reads multicolumn data

plot(dat[0],dat[1])
show()
