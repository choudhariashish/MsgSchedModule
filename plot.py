#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

data = open("MsgSchedOutput.ods").read().split('\n')
data = data[:-1]

x = [row.split('\t')[0] for row in data]
y1 = [row.split('\t')[1] for row in data]
y2 = [row.split('\t')[2] for row in data]
y3 = [row.split('\t')[3] for row in data]


fig = plt.figure()

ax1 = fig.add_subplot(111)

ax1.set_title("Time Vs. RTT")    
ax1.set_xlabel('Time')
ax1.set_ylabel('RTT')

ax1.plot(x,y1, c='r', label='Period')
ax1.plot(x,y2, c='b', label='Curr_K')
ax1.plot(x,y3, c='g', label='Actual_RTT')

leg = ax1.legend()

plt.show()
