#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import sys
import csv

sys.argv

total = len(sys.argv)

print total

for i in range(0, total):
	print sys.argv[i]

filename = open(sys.argv[1], 'r')

data = filename.read().split('\n')
#data = open('MSMOutput_ECN1_DELTANV10.lgdat').read().split('\n')
data = data[3:-1]

x_time = [row.split(';')[0] for row in data]
y_period = [row.split(';')[1] for row in data]
y_currk = [row.split(';')[2] for row in data]
y_rtt = [row.split(';')[3] for row in data]
y_pa = [row.split(';')[4] for row in data]
#y_ecn = [row.split(';')[5] for row in data]

for i in range(0, len(y_currk)):
	y_currk[i] = 50 + (int)(y_currk[i])*5;
#	y_ecn[i] = 150 * (int)(y_ecn[i]);	

fig = plt.figure()

fig, ((ax1), (ax2)) = plt.subplots(nrows=2, ncols=1)

#ax1 = fig.add_subplot(111)
ax1.set_title("No ECN and With Traffic \n Time Vs. RTT,Curr_K,Period")    
ax1.set_xlabel('Time')
ax1.set_ylabel('RTT,Curr_K,Period')
ax1.plot(x_time,y_period, c='r', label='Period')
ax1.plot(x_time,y_currk, c='b', label='Curr_K')
ax1.plot(x_time,y_rtt, c='g', label='RTT')
#ax1.plot(x_time,y_ecn, c='g', label='ECN')
leg = ax1.legend()

#ax2 = fig.add_subplot(223)
ax2.set_title("Time Vs. PA (Delta Vary->10 to 100)")    
ax2.set_xlabel('Time')
ax2.set_ylabel('Power Acknowledged')
ax2.plot(x_time,y_pa, c='b', label='PA')
leg = ax1.legend()


plt.show()
