#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import sys

sys.argv

total = len(sys.argv)

for i in range(0, total):
	print sys.argv[i]

data = open(sys.argv[0]).read().split('\n')
data = data[:3]
data = data[:-1]

x_time = [row.split(';')[0] for row in data]
y_period = [row.split(';')[1] for row in data]
y_currk = [row.split(';')[2] for row in data]
y_rtt = [row.split(';')[3] for row in data]


fig = plt.figure()

ax1 = fig.add_subplot(111)

ax1.set_title("Time Vs. RTT")    
ax1.set_xlabel('Time')
ax1.set_ylabel('RTT')

ax1.plot(x_time,y_period, c='r', label='Period')
ax1.plot(x_time,y_currk, c='b', label='Curr_K')
ax1.plot(x_time,y_rtt, c='g', label='Actual_RTT')

leg = ax1.legend()

plt.show()
