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


def make_graph(percents,  # the data to graph
               rows_k=range(3,13),  # which rows to include
               cols_n=range(0,15),  # which columns to include
               xtics=None,          # custom values to have xtick marks
               curve_per_row=True,  # each row is one curve
               lin_scale=True, # either/both
               log_scale=True, 
               legend=True,
               auto_legend=False,
               annotate=False):

    # what's the smallest percent that will be graphed?
    # we need this to set the log scale
    minpct = 1.0
    for k in rows_k:
        for n in cols_n:
            if pcts[k][n]:
                minpct = min(percents[k][n], minpct)
    power = math.log10(minpct)
    miny = 10 ** (math.floor(power) - 1)

    # one graph or two?
    nsubplots  = 1 if lin_scale else 0
    nsubplots += 1 if log_scale else 0
    if nsubplots==0:  # !!!
        return

    # two subplots, 2x1 means above/below, in 1 column    
    fig,ax = plt.subplots(nsubplots,1)

    fig.set_size_inches(16,9)

    if nsubplots==2:
        lin_ax = ax[0]
        log_ax = ax[1]
    else:
        if lin_scale: lin_ax = ax
        else:         log_ax = ax
    top_ax = lin_ax if lin_scale else log_ax

    # very light gray ledger lines in the log graph
    log_labels = []
    if log_scale:
        for exp in range(10):
            y = 10**(-exp)
            xs = cols_n if curve_per_row else rows_k
            log_ax.semilogy((xs[0],xs[-1]), (y,y), color='#dddddd', label=None)
            log_labels.append(None)

    # graph the curves
    curve_per_col = not curve_per_row
    ncurves = len(rows_k) if curve_per_row else len(cols_n)
    for curve in range(ncurves):

        if curve_per_row:
            xs = cols_n
            ys = zero2nan(percents[rows_k[curve], cols_n], len(cols_n))
        else:
            xs = rows_k
            ys = zero2nan(percents[rows_k, cols_n[curve]], len(rows_k))

        fmt = '-o'
        if curve_per_row and curve>12:
            fmt = '--o' # dashed lines for randomly sampled k>12 curves

        if legend:
            if curve_per_row: label = '{} cards'.format(rows_k[curve])
            else:             label = '{} SETs' .format(cols_n[curve])
        else:
            label = None

        # This is where the curve is actually plotted
        if lin_scale: lin_ax.plot    (xs, ys, fmt, label=label)
        if log_scale: log_ax.semilogy(xs, ys, fmt, label=label)
        
    # X axis formatting
    xlabel = 'Number of SETs' if curve_per_row else 'Number of cards'
    if log_scale: log_ax.set_xlabel(xlabel)
    else:         lin_ax.set_xlabel(xlabel)

    if xtics is None:
        if curve_per_row: xtics = range(0,15)
        else:             xtics = range(3,13,3)
    if lin_scale: lin_ax.set_xticks(xtics)
    if log_scale: log_ax.set_xticks(xtics)

    # apply the y range if we have a log scale
    if log_scale:
        log_ax.set_ylim([miny,1.8])

    # draw a dividing line between enumerated and sampled    
    if curve_per_col and rows_k[-1] > 12:
        if lin_scale:
            lin_ax.plot([12.5,12.5], [0,1],     '-', color='#aaaaaa', linewidth=3)
        if log_scale:
            log_ax.plot([12.5,12.5], [miny,1.8],'-', color='#aaaaaa', linewidth=3)
        top_ax.annotate('enumerated', (11.5, 0.9))
        top_ax.annotate('sampled',    (12.6, 0.9))

    if legend:
        if auto_legend: # legend inside UR, eliminate margins
            plt.subplots_adjust(left=.03, bottom=.05, right=0.99, top=0.99, hspace=0.1)
            top_ax.legend(loc='upper right')
        else: # Minimize most of the margins, except leave a wide margin on the right for the legend
            plt.subplots_adjust(left=.03, bottom=.05, right=0.9,  top=0.99, hspace=0.1)
            top_ax.legend(prop={'size': 14}, loc=(1.01, -0.3))
    else: # no legend, push the right margin all the way out
        plt.subplots_adjust(left=.03, bottom=.05, right=0.99, top=0.99, hspace=0.1)

    if annotate and curve_per_col:
        for n in cols_n:
            for k in rows_k:
                if percents[k][n] > 0:
                    if   n in (25,30,32,34,40,46): pos = (k+.05, percents[k][n]*0.45)
                    elif n == 28:                  pos = (k-.1,  percents[k][n]*0.45)
                    elif n <  10:                  pos = (k-.2,  percents[k][n]*0.7)
                    else:                          pos = (k-0.25,percents[k][n]*0.7)
                    break

            if   n==0:
                if lin_scale: lin_ax.annotate('0 SETs', (2.75,0.9))
                if log_scale: log_ax.annotate('0 SETs', (2.75,percents[3][0]*0.3))
            elif n==1:
                if lin_scale: lin_ax.annotate('1 SET',  (2.75,0.05))
                if log_scale: log_ax.annotate('1 SET',  (2.75,percents[3][1]*0.3))
            else:
                if log_scale: log_ax.annotate(str(n), pos)
        
    plt.show()
        

        

    
    
################################################################################    
################################################################################    
####################         Main Program starts here       ####################
################################################################################    
################################################################################    
    

# read and normalize the data

# first create 3 empty rows
ncols = 47
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


data = np.array(listofrows)
#print('data size is', data.shape)

# normalize as percentages and compute min pct (for graph y ranges)
pcts = np.array(listofrows, 'float64')
tots = np.transpose(np.array(rowsums, 'float64'))
for i in range(3,len(listofrows)):
    for j in range(ncols):
        pcts[i][j] = data[i][j] / tots[i]




# edit these to control what portion of data is graphed
rowsk = range(3,13)     # range(3,13) actually creates the list [3,4,5,6,7,8,9.10,11,12]
ktics = range(3,13,3)   # x-axis ticks at 3, 6, 9, 12

colsn = range(0,15)     # default [0,1,...14], because 12 cards has max 14 SETs
ntics = range(0,15,2)   # x-axis ticks at 0,2,...14

# uncomment these to get fuller graphs including sampled k=13..21
rowsk = range(3,22)
ktics = (3,6,9,12,15,18,21) # Number of cards in an actual game can only be multiples of 3
#ktics = (3,12,21)           # Min/Start/Max
colsn = range(0,ncols)      # all possible nSETs




make_graph(pcts)
make_graph(pcts, curve_per_row=False)


ntics = list(range(0,ncols,5))
ntics.append(ncols-1)
make_graph(pcts,
           rows_k=range(3,22),
           cols_n=range(0,ncols),
           xtics=ntics)

make_graph(pcts,
           rows_k=range(3,22),
           cols_n=range(0,ncols),
           curve_per_row=False,
           xtics=range(3,22,3),
           legend=False,
           annotate=True)

xs = range(2)
make_graph(pcts, rows_k=range(3,4), cols_n=xs, xtics=xs, log_scale=False, auto_legend=True)
xs = range(2)
make_graph(pcts, rows_k=range(3,5), cols_n=xs, xtics=xs, log_scale=False, auto_legend=True)
xs = range(3)
make_graph(pcts, rows_k=range(3,6), cols_n=xs, xtics=xs, log_scale=False, auto_legend=True)
xs = range(4)
make_graph(pcts, rows_k=range(3,7), cols_n=xs, xtics=xs, log_scale=False, auto_legend=True)
xs = range(6)
make_graph(pcts, rows_k=range(3,8), cols_n=xs, xtics=xs, log_scale=False, auto_legend=True)
make_graph(pcts, rows_k=range(3,8), cols_n=xs, xtics=xs)
make_graph(pcts, rows_k=range(3,8), cols_n=xs, xtics=xs, lin_scale=False, auto_legend=True)
xs = range(9)
make_graph(pcts, rows_k=range(3,9), cols_n=xs, xtics=xs, lin_scale=False, auto_legend=True)
xs = range(13)
make_graph(pcts, rows_k=range(3,10),cols_n=xs, xtics=xs, lin_scale=False, auto_legend=True)
xs = range(13)
make_graph(pcts, rows_k=range(3,11),cols_n=xs, xtics=xs, lin_scale=False, auto_legend=True)
xs = range(14)
make_graph(pcts, rows_k=range(3,12),cols_n=xs, xtics=xs, lin_scale=False, auto_legend=True)
xs = range(15)
make_graph(pcts, lin_scale=False, auto_legend=True)
make_graph(pcts)


    


