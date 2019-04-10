import numpy as np
import matplotlib.pyplot as plt
import sys
from math import sqrt, atan2
import time

my_filter_address="8C:0C:90:01:1E:C8"
my_filter_length=384

f = open("mycapture.txt", "r")
lu=f.read()
loop=0
for line in lu.split("\n"):
	if len(line)>10:
		if "<CSI>" in line:
			#print line
			address=(line.split("<address>")[1]).split("</address>")[0]
			length=(line.split("<len>")[1]).split("</len>")[0]
			
			if address==my_filter_address:
				if int(length)==my_filter_length:
					print address, "@", length, "@"
					data=(line.split("</len>")[1]).split()
					#print data
					list_im=[]
					list_real=[]
					try:
						for i in range(len(data)):
							if i%2==0:
								list_im.append(int(data[i]))
							else:
								list_real.append(int(data[i]))
					except Exception as e:
						print e
						continue
					
					list_amp=[]
					list_ang=[]
					for i in range(len(data)/2):
						list_amp.append(sqrt(list_im[i]**2+list_real[i]**2))
						list_ang.append(atan2(list_im[i], list_real[i]))
							
					if loop>=4*4:
						break
						
					plt.subplot(4,4,loop+1)
					plt.title("Phase for frame"+str(loop))
					plt.plot(list_ang[0:64])

					
					loop+=1
plt.show(block=True)
						
			
