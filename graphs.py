#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import csv

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

# plot each row as a line
for rowi in rowis:
   plt.plot(colis, pcts[rowi, colis])
plt.show()

for coli in colis:
   plt.plot(rowis, pcts[rowis, coli])
plt.show()

timex = [0]*4
for i in range(4,13):
   timex.append(seconds[i]/seconds[i-1])
plt.plot(range(4,13), timex[4:13])
plt.show()
