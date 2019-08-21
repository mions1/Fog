#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from scipy.stats import norm
from scipy.stats import lognorm
from scipy.stats import gamma
from scipy.optimize import curve_fit
import vectorParse as vp
import sys
from os import path

def myfilter(x, med, thr):
    if np.abs(x-med) < thr:
        return x
    else:
        return np.nan


def gauss(x, mu, sigma, scale):
    #return scale * np.exp(-0.5 * ((x-mu)/sigma) * ((x-mu)/sigma))
    l = (x-mu)/sigma
    return scale * norm.pdf(l)


def lognormal(x, mean, sigma, scale):
    #var = sigma * sigma
    #mu = np.log(mean/np.sqrt(1+var/(mean*mean)))
    #sigma2 = np.log(1 + var / (mean * mean))
    #return scale * .1/x * np.exp((-0.5/sigma2) * (np.log(x)-mu) * (np.log(x)-mu))
    l = (x-mean)/sigma
    s = sigma
    return scale * lognorm.pdf(l, s)


def mygamma(x, mean, a, scale):
    #var = sigma * sigma
    #mu = np.log(mean/np.sqrt(1+var/(mean*mean)))
    #sigma2 = np.log(1 + var / (mean * mean))
    #return scale * .1/x * np.exp((-0.5/sigma2) * (np.log(x)-mu) * (np.log(x)-mu))
    l=x-mean
    return scale * gamma.pdf(l, a)


def gauss2(x, mu1, sigma1, mu2, sigma2, ratio):
    global scale
    scale1 = scale*ratio
    scale2 = scale*(1-ratio)
    return gauss(x, mu1, sigma1, scale1)+gauss(x, mu2, sigma2, scale2)

dir = sys.argv[1]	#directory dei file .vec
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#directory dove salvare i disegni (=dir se non passata)

vecFiles = vp.getVec(dir,"vec")
dictRun = vp.divideByRun(vecFiles)
values = vp.getValues(dictRun, dir)
vp.makeFile(values, dir2)

files = vp.getFiles(dir2,"vp")

for file in files:
    data = pd.read_csv(path.join(dir2,file), sep="\t", header=None)
    data.columns = ['value']
    d = data['value']

    med = d.median()
    iqr = d.quantile(q=0.75) - d.quantile(q=0.25)
    mean = d.mean()
    std = d.std()

    cent = med
    var = iqr

    # filter data
    dfilt = d.apply(myfilter, args=(cent, 5*var))
    dfilt = dfilt.dropna()
    #dfilt = d

    # different number of bins
    # (c1, e1) = np.histogram(dfilt.values, bins='scott')
    # (c2, e2) = np.histogram(dfilt.values, bins='fd')
    # (c3, e3) = np.histogram(dfilt.values, bins='sturges')
    (c, e) = np.histogram(dfilt.values, bins='sturges')
    # normalize as probability density
    c = c / dfilt.size
    e1 = np.empty(e.size-1)
    for i in range(0, e.size-1):
        e1[i] = (e[i]+e[i+1])/2
    (fig, axes) = plt.subplots(ncols=1, nrows=2, figsize=[10.24, 7.68])
    a1 = dfilt.plot.box(ax=axes[0], vert=False)
    # a2 = dfilt.plot.hist(ax=axes[1], bins=e)
    axes[1].bar(e1, c, (e[1]-e[0])*.8)
    
    plt.savefig(path.join(dir2, *["plot", file + "_1.png"]))
    plt.clf()
    #plt.show()

