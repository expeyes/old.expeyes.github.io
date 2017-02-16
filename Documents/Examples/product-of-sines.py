from pylab import *

t = linspace(0, .01, 500)  # array of time, the independent variable

f = 200   
y1 = sin(2*pi*f*t)     # 200 Hz sine wave, the signal

f = 5000 
y2 = sin(2*pi*f*t)     # 5 kHz sine wave, the carrier

plot(t, y1)
plot(t, y1 * y2)       # product of y1 and y2
xlabel('time (sec)')
show()
