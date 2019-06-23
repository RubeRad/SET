#! /usr/bin/env python3

import csv                      # for reading csv files
import math                     # log base 10
import numpy as np              # all kinds of numeric capabilities
import matplotlib.pyplot as plt # creating graphs (customized for presentation within a jupyter notebook)

def zero2nan(haszeros, outlen):
    hasnans = []
    for num in haszeros:
        if num==0: hasnans.append(np.nan)
        else:      hasnans.append(num)
    nans = list(np.isnan(hasnans))
    while len(nans) and nans[-1]:
        nans.pop()
        hasnans.pop()
    while len(hasnans) < outlen:
        hasnans.append(np.nan)
    return hasnans

# set this True or False if you want more printouts 
verbose = False

# edit these to control what portion of data is graphed
ncols = 47
rowsk = range(3,13)     # range(3,13) actually creates the list [3,4,5,6,7,8,9.10,11,12]
ktics = range(3,13,3)   # x-axis ticks at 3, 6, 9, 12

colsn = range(0,15)     # default [0,1,...14], because 12 cards has max 14 SETs
ntics = range(0,15,2)   # x-axis ticks at 0,2,...14

# uncomment these to get fuller graphs including sampled k=13..21
rowsk = range(3,22)
ktics = (3,6,9,12,15,18,21) # Number of cards in an actual game can only be multiples of 3
#ktics = (3,12,21)           # Min/Start/Max
colsn = range(0,ncols)      # all possible nSETs
ntics = list(range(0,ncols,5)) # don't crowd the x axis
ntics.append(ncols-1)

# first create 3 empty rows
listofrows = []
for i in range(3):
   listofrows.append([0]*ncols)  # now we have 3x47 full of zeroes
rowsums = [0]*3                  # (0,0,0) because we don't need rowsums[0..2]
seconds = [0]*3

# then read in data rows starting with k=3 from the data file
with open('enumerate_deals.csv') as csvfile:
    rdr = csv.reader(csvfile)
    for row in rdr:
        if row[0] == 'N': # skip the header
            continue
        rowsums.append( int (row[1]))
        seconds.append(float(row[2]))
        if verbose:
            print('k=', row[0], 'Rowsum:', rowsums[-1], 'seconds:', seconds[-1])
        nums = []
        for i in range(ncols):
            if len(row) > i+3:
                strval = row[i+3]
                if strval == '':
                    nums.append(0)
                else:
                    nums.append(int(row[i+3]))
            else:
                nums.append(0)
        listofrows.append(nums)
        if verbose:
            print(nums)


data = np.array(listofrows)
print('data size is', data.shape)

# normalize as percentages and compute min pct (for graph y ranges)
pcts = np.array(listofrows, 'float64')
tots = np.transpose(np.array(rowsums, 'float64'))
for i in range(3,len(listofrows)):
    for j in range(ncols):
        pcts[i][j] = data[i][j] / tots[i]

minpct = 1.0
for k in rowsk:
    for n in colsn:
        if pcts[k][n]:
            minpct = pcts[k][n]
power = math.log10(minpct)
miny = 10 ** (math.floor(power) - 1)
if verbose:
    print('minpct {:.2e} miny {}'.format(minpct, miny))


# per-k curves, x axis is nSETs
fig,ax = plt.subplots(2,1) # two subplots, 2x1 means above/below, in 1 column

# very light gray ledger lines in the log graph
for exp in range(10):
   ax[1].semilogy((0,colsn[-1]), (10**(-exp), 10**(-exp)), color='#dddddd' )

for rowi in rowsk:
    # zero2nan replaces 0 with nan so linespoints will graph appropriately with gaps
    ys = zero2nan(pcts[rowi, colsn], len(colsn))
    # -o means lines and points, --o is dashed, colors are automatically chosen
    fmt = '--o' if rowi>12 else '-o'
    ax[0].plot    (colsn, ys, fmt) # ax[0] is top graph;   plot()   does lin scale
    ax[1].semilogy(colsn, ys, fmt) # ax[1] is bot graph; semilogy() does log scale

# deal with formatting issues
if len(ntics) > 0 and ntics[-1] <= colsn[-1]: # if desired x-axis tick marks are specified
    ax[0].set_xticks(ntics)
    ax[1].set_xticks(ntics)
ax[1].set_xlabel('Number of SETs') # label the lower x axis (applies to both)
ax[1].set_ylim([miny,1.8])      # set the y-range of the log graph

# Don't let the legend get out of control. Omit if too large
plt.subplots_adjust(left=.05, bottom=.1, right=0.85, top=0.99, hspace=0.1)
labelsk = []
for i in rowsk:
    labelsk.append('{} cards'.format(i))
if len(labelsk) <= 20:
    # Minimize most of the margins, except leave a wide margin on the right for the legend
    plt.subplots_adjust(left=.03, bottom=.05, right=0.9,  top=0.99, hspace=0.1)
    ax[0].legend(labelsk, prop={'size': 14}, loc=(1.01, -0.3))
else:
    # no legend, push the right margin all the way out
    plt.subplots_adjust(left=.03, bottom=.05, right=0.99, top=0.99, hspace=0.1)

plt.show()


# per-nSETs curves, x axis is number of cards
fig,ax = plt.subplots(2,1)

# very light gray ledger lines in the log graph
for exp in range(10):
   ax[1].semilogy((rowsk[0],rowsk[-1]), (10**(-exp), 10**(-exp)), color='#dddddd' )

for coli in colsn:                                 # for columns, not rows
   ys = zero2nan(pcts[rowsk, coli], len(rowsk))    # each list of ys is all rows for a particular col
   ax[0].plot    (rowsk, ys, '-o')
   ax[1].semilogy(rowsk, ys, '-o')

 # draw a dividing line between enumerated and sampled    
if rowsk[-1] > 12:
   ax[0].plot([12.5,12.5], [0,1],     '-', color='#aaaaaa', linewidth=3)
   ax[1].plot([12.5,12.5], [miny,1.8],'-', color='#aaaaaa', linewidth=3)
   ax[0].annotate('enumerated', (11.5, 0.9))
   ax[0].annotate('sampled',    (12.6, 0.9))

if len(ktics) > 0 and ktics[-1]<=rowsk[-1]:        # mark X axis with ktics
    ax[0].set_xticks(ktics)
    ax[1].set_xticks(ktics)
ax[1].set_xlabel('Number of cards')                # different X axis label
ax[1].set_ylim([miny,1.8])



labelsn = []
for n in colsn:
    labelsn.append('{} SETs'.format(n))            # different legend labels
if len(labelsn) <= 20:
    ax[0].legend(labelsn, prop={'size': 14}, loc=(1.01, -0.5))
    plt.subplots_adjust(left=.03, bottom=.05, right=0.9,  top=0.99, hspace=0.1)
else: # 
    plt.subplots_adjust(left=.03, bottom=.05, right=0.99, top=0.99, hspace=0.1)
    # annotate each curve
    for coli in colsn:
      # find first non-nan
      for k in rowsk:
         if pcts[k][coli] > 0:
            if   coli in (25,30,32,34,40,46): pos = (k+.05, pcts[k][coli]*0.45)
            elif coli == 28:                  pos = (k-.1,  pcts[k][coli]*0.45)
            elif coli <  10:                  pos = (k-.2,  pcts[k][coli]*0.7)
            else:                             pos = (k-0.25,pcts[k][coli]*0.7)

            if   coli==0:
               ax[0].annotate('0 SETs', (2.75,0.9))
               ax[1].annotate('0 SETs', (2.75,pcts[3][0]*0.3))
            elif coli==1:
               ax[0].annotate('1 SET',  (2.75,0.05))
               ax[1].annotate('1 SET',  (2.75,pcts[3][1]*0.3))
            else:
               ax[1].annotate(str(coli), pos)
      
            break;


plt.show()
