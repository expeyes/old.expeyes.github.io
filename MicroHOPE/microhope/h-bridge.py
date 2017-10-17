import time
from pymicro import *
p=atm32()

p.outb(DDRA,15)
while 1:
	val = input('Enter 0(stop motor), 1 or 2 : ')
	p.outb(PORTA,val)  

