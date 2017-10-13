from pylab import *

t = linspace(0, 1, 500)  # array of time, the independent variable

f = 100     
y1 = sin(2*pi*f*t)     # 100 Hz sine wave

f = 104     
y2 = sin(2*pi*f*t)     # 104 Hz sine wave

plot(t, y1 + y2)
show()
