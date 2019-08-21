#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import plot as p
from scipy.stats import norm
from scipy.stats import lognorm
from scipy.stats import gamma
from scipy.optimize import curve_fit
import vectorParse as vp
import scaParse as sp
import sys, glob, os
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

"""
Crea una lista di file mettendo i file che rispettano certe restrizioni
"""
def together(files, name, postfix=""):
    filtfiles = []
    for file in files:
        if "VS" in file and name in file[:file.index("VS")] and name in file[file.index("VS"):]:
            if postfix != "":
                if file[-len(postfix):] == postfix:
                    filtfiles.append(file)
            else:
                filtfiles.append(file)
    return filtfiles

def deleteFiles(files, dir="."):
    for file in files:
        try:
            if "rho_02" not in file and "mul_001" not in file:
                os.remove(path.join(dir,file))
        except OSError as error:
            print(error)
            print("Error while deleting")
        
def cleanDir(regex):
    fileList = glob.glob(regex, recursive=False)
    for file in fileList:
        try:
            os.remove(file)
        except OSError:
            print("Error while deleting")

def makeFile(dir1, dir2, dict, valuesName, preset=1):
    values = sp.getValue(dict, dir1, valuesName)  # Prendo i valori che mi servono (es. responseTime)
    means = sp.getMeans(values)  # Prendo le medie per run
    sp.makeFile(means, path.join(dir2, split[-2]), valuesName, preset)  # crea i file .sp in dir2

"""
Unisce i file di cui vogliamo fare i grafici uniti
Creando un file a 3 colonne (es. rho(x) - values1(y1) - values2(y2))
"""
def makeFiles1VSFiles2(files1, files2, dir2, valuesFile1, valuesFile2, delete=False):
    files1.sort()
    files2.sort()

    sp.joinFiles(dir2,files1,files2)

    #Recupero i files appena creati
    filesJoin = sp.getFiles(dir2, valuesFile2+"_joined.sp")

    #Plot dei grafici uniti
    #drawOverlayed(dir2, together(filesJoin,"mul_1","joined.sp"), "_1", valuesFile1, valuesFile2)
    #drawOverlayed(dir2, together(filesJoin,"mul_001","joined.sp"), "_001", valuesFile1, valuesFile2)
    if (delete):
        deleteFiles(filesJoin, dir2)
    
    return filesJoin
    
"""
Sostanzialmente fa quello che fa makeFilesNoLB ma in più deve normalizzare i valori di droppedJobs
"""
def makeDroppedJobs(dir, dir2, dictRun, preset=1):

    values1=sp.getValue(dictRun, dir, "droppedJobsTotal")
    values2=sp.getValue(dictRun, dir, "totalJobs")
    values3 = dict()
    for key in values1:
        if key not in values3:
            values3[key] = list()
        for i in range(len(values1[key])):
            if values2[key][i] == 0:
                values3[key].append(0)
            else:
                values3[key].append(values1[key][i]/values2[key][i])
    means = sp.getMeans(values3)
    sp.makeFile(means, path.join(dir2, split[-2]), "droppedJobsTotalfrattoTotalJobs", preset=preset)
    files = sp.getFiles(dir2, "frattoTotalJobs.sp")
    #drawSingle(dir2, files, "rho", "droppedJobs")
    #drawDouble(dir2, together(files,"mul_001"),"_001;droppedJobs",x_label="rho",y_label="droppedJobs",start="MG",end=";")
    #drawDouble(dir2, together(files,"mul_1"),"_1;droppedJobs",x_label="rho",y_label="droppedJobs",start="MG",end=";")
    return files 
