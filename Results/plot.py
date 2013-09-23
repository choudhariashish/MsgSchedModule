#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import sys
import csv

sys.argv

total = len(sys.argv)

for i in range(0, total):
	print sys.argv[i]

#data = open(str(sys.argv[0])).read().split('\n')
data = open('MSMOutput.lgdat').read().split('\n')
data = data[3:-1]

x_time = [row.split(';')[0] for row in data]
y_period = [row.split(';')[1] for row in data]
y_currk = [row.split(';')[2] for row in data]
y_rtt = [row.split(';')[3] for row in data]
y_perror = [row.split(';')[4] for row in data]
y_ecn = [row.split(';')[5] for row in data]

for i in range(0, len(y_currk)):
	y_currk[i] = 50 + (int)(y_currk[i])*5;
	y_perror[i] = 50 + (int)(y_perror[i]);
	y_ecn[i] = 150 * (int)(y_ecn[i]);	

fig = plt.figure()

ax1 = fig.add_subplot(111)

ax1.set_title("Time Vs. RTT,Curr_K,Period,Perror,ECN")    
ax1.set_xlabel('Time')
ax1.set_ylabel('RTT,Curr_K,Period,Perror,ECN')

ax1.plot(x_time,y_period, c='r', label='Period')
ax1.plot(x_time,y_currk, c='b', label='Curr_K')
ax1.plot(x_time,y_rtt, c='g', label='RTT')
ax1.plot(x_time,y_perror, c='b', label='Perror')
ax1.plot(x_time,y_ecn, c='g', label='ECN')

leg = ax1.legend()

plt.show()
