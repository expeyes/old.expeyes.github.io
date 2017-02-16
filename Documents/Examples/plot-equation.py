from pylab import *

x = linspace(0, 10*pi, 300)  # 300 point array, from o to 2pi
y = sin(x) 		# sine function of numpy can handle arrays
plot(x,y)		# Read Matplotlib documentation
xlabel('angle')
ylabel('sine(x)')
show()
