from pylab import *

N = 101
a = linspace(-5, 5, N)
b = linspace(-5, 5, N)
xa, ya = meshgrid(a, b)		# A meshgrid in the xy plane. xa contains the x-coordinates
Ex = zeros_like(xa)			# 2D array to store the Ex and
Ey = zeros_like(ya)			# Ey components

Q = [(-15, 0, 3), (5, -2, -2), (-5, 2, -2)]	# Charges (Value, x-cord, y-cord)

for q in Q:  # mark charge locations
	text(q[1], q[2], 'o', color = 'r', fontsize=15, va='center', ha='center')

for i in range(N):		# calculate Ex and Ey at each point in the grid, due to all charges
	for j in range(N):
		x = xa[i,j]
		y = ya[i,j]
		for k in range(len(Q)): # sum over the charges, using equation from the text book
			Ex[i,j] += Q[k][0]*(x-Q[k][1])/ ((x-Q[k][1])**2+(y-Q[k][2])**2)**(1.5)
			Ey[i,j] += Q[k][0]*(y-Q[k][2])/ ((x-Q[k][1])**2+(y-Q[k][2])**2)**(1.5)

streamplot(xa, ya, Ex, Ey, density= 1.6)
show()
