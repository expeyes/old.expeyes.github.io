#Plot n-curves. Run the code for different values of n.

from pylab import  *

n = 2			
t = linspace(0, 1, 3000)

x = cos(2*pi*n*t) + cos(2*pi*t) -1
y = sin(2*pi*n*t) + sin(2*pi*t)

plot(x,y)
show()


