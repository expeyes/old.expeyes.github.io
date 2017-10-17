from pylab import *

t = 0.0         # Stating time
R = .0
L = 1.0
C = 1.0

dt = 0.01       # value of time increment
N = 10000
ta = zeros(N)
va = zeros(N)
ia = zeros(N)
ta[0] = 0
va[0] = 5.0
ia[0] = .0

for k in range(N-1):
	ta[k+1] = (k+1)*dt
	ia[k+1] = ia[k] + ( -R/L * ia[k] - (1.0/L) * va[k])*dt
	va[k+1] = va[k] + (1.0/C) * ia[k] *dt
	
plot(ta,va)
plot(ta,ia)
show()
