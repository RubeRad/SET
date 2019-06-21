#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import csv

def zero2nan(haszeros):
   hasnans = []
   for num in haszeros:
      if num==0: hasnans.append(np.nan)
      else:      hasnans.append(num)
   nans = list(np.isnan(hasnans))
   while nans[-1]:
      nans.pop()
      hasnans.pop()
   return hasnans


listofrows = []
for i in range(3):
   listofrows.append([0]*15)
rowsums = [0]*3
seconds = [0]*3
with open('enumerate_deals.csv') as csvfile:
   rdr = csv.reader(csvfile)
   for row in rdr:
      if row[0] == 'N': # skip the header
         continue
      rowsums.append(int(row[1]))
      seconds.append(float(row[2]))
      nums = []
      for i in range(15):
         if len(row) > i+3:
            strval = row[i+3]
            if strval == '':
               nums.append(0)
            else:
               nums.append(int(row[i+3]))
         else:
            nums.append(0)
      listofrows.append(nums)


data = np.array(listofrows)
pcts = np.array(listofrows, 'float64')
tots = np.transpose(np.array(rowsums, 'float64'))
for i in range(3,13):
   for j in range(15):
      pcts[i][j] = data[i][j] / tots[i]

#print(data)
#print(pcts)

rowis = range(3,13)
colis = range(0,15)

labels1 = []
for i in range(3, 13):
   labels1.append('{} cards'.format(i))

labels2 = ['0 SETs', '1 SET']
for i in range(2,15):
   labels2.append('{} SETs'.format(i))

# plot each row as a line
fix,ax = plt.subplots(2,1)
for rowi in rowis:
   ys = zero2nan(pcts[rowi, :])
   ax[0].plot(range(len(ys)), ys, '-o')
ax[0].legend(labels1, prop={'size': 14}, loc=(1.01, -0.3))
#plt.show()


# now log scale
for rowi in rowis:
   ys = zero2nan(pcts[rowi, :])
   ax[1].semilogy(range(len(ys)), ys, '-o')
ax[1].set_ylim([8.0e-10,1.8])
ax[1].set_xlabel('Number of SETs')
#plt.legend(labels1)
plt.subplots_adjust(left=.03, bottom=.03, right=0.9, top=0.99, wspace=0, hspace=0.1)
#mng = plt.get_current_fig_manager()
#mng.window.showMaximized()
plt.show()

fig,ax = plt.subplots(2,1)
for coli in colis:
   ys = zero2nan(pcts[:, coli])
   ax[0].plot(range(len(ys)), ys, '-o')
ax[0].legend(labels2, prop={'size': 14}, loc=(1.01, -0.5))
#plt.show()

for coli in colis:
   ys = zero2nan(pcts[:, coli])
   ax[1].semilogy(range(len(ys)), ys, '-o') # how does this know X axis is 3... not 0...?
ax[1].set_ylim([8.0e-10,1.8])
ax[1].set_xlabel('Number of cards')
#plt.legend(labels2)
plt.subplots_adjust(left=.03, bottom=.03, right=0.9, top=0.99, wspace=0, hspace=0.1)
#mng = plt.get_current_fig_manager()
#mng.frame.Maximize(True)
plt.show()
exit(0)

timex = [0]*4
for i in range(4,13):
   timex.append(seconds[i]/seconds[i-1])
plt.plot(range(4,13), timex[4:13])
plt.show()
