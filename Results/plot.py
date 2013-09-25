#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import sys
import csv

TICK_Reso=0
RTM=1
RTT_Ctr=2
Kmax=3
ECN=4
DELTA_vary=5
DELTA_MIN=6
DELTA_MAX=7
Total_PT=8


sys.argv
total = len(sys.argv)
for i in range(0, total):
	print sys.argv[i]

filename = open(sys.argv[1], 'r')

data = filename.read().split('\n')

filename.close()

# get first 10 lines because it has 
# configuration data
config_data = data[1:10]
config=[]
for line in config_data:
	columns = line.split(' ')
	print columns[-1]
	config += [int(columns[-1])]



# exclude first 12 lines because it has 
# configuration data and LiveGraph stuff
data = data[12:-1]

x_time = [row.split(';')[0] for row in data]
y_period = [row.split(';')[1] for row in data]
y_currk = [row.split(';')[2] for row in data]
y_rtt = [row.split(';')[3] for row in data]
y_pa = [row.split(';')[4] for row in data]

if config[ECN]:
	y_ecn = [row.split(';')[5] for row in data]

for i in range(0, len(y_currk)):
	y_currk[i] = 50 + (int)(y_currk[i])*5;
	if config[ECN]:	
		y_ecn[i] = 150 * (int)(y_ecn[i]);	

fig = plt.figure()
fig, ((ax1), (ax2)) = plt.subplots(nrows=2, ncols=1)

#ax1 = fig.add_subplot(111)

if config[ECN]:
	ax1.set_title("Time Vs. RTT,Curr_K,Period,ECN")    
else:
	ax1.set_title("Time Vs. RTT,Curr_K,Period")    

ax1.set_xlabel('Time')

if config[ECN]:
	ax1.set_ylabel('RTT,Curr_K,Period,ECN')
else:
	ax1.set_ylabel('RTT,Curr_K,Period')

ax1.plot(x_time,y_period, c='r', label='Period')
ax1.plot(x_time,y_currk, c='b', label='Curr_K')
ax1.plot(x_time,y_rtt, c='g', label='RTT')

if config[ECN]:
	ax1.plot(x_time,y_ecn, c='g', label='ECN')

leg = ax1.legend()

#ax2 = fig.add_subplot(223)
if config[DELTA_vary]:
	ax2.set_title("Time Vs. PA (DeltaMin=%d, DeltaMax=%d )" % (config[DELTA_MIN],config[DELTA_MAX]))    
else:
	ax2.set_title("Time Vs. PA (Delta=%d)" % config[DELTA_MIN])    

ax2.set_xlabel('Time')
ax2.set_ylabel('Power Acknowledged')
ax2.plot(x_time,y_pa, c='b', label='PA')
leg = ax1.legend()

plt.show()

fig.savefig('ecn_%d_delta_%d.png' % (config[ECN],config[DELTA_vary]))
