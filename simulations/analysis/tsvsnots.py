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
import analysis_sca as ansca
import sys, glob, os
from os import path

def tmp(dir, file, name, x_label="", y_label=""):
    data = sp.getListFromFile(path.join(dir, file),z=True)
    plt.plot(data["x"], data["y"], 'r-', label="time-sharing")  # , t, t ** 2, 'bs', t, t ** 3, 'g^')
    plt.plot(data["x"], data["z"], 'b--', label="No time-sharing")

    #plt.title(name)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.savefig(path.join(dir, *["plot", name + ".svg"]))
    plt.clf()
    pass

"""
#TSVSNOTS
"""

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

respFiles = sp.getFiles(dir,"responseTime.jsp")
dropFiles = sp.getFiles(dir,"droppedJobs.jsp")

tmp(dir, respFiles[0], "TsVSNoTs-Mul_1-responseTime" ,"rho", "responseTime")
tmp(dir, dropFiles[0], "TsVSNoTs-Mul_1-droppedJobs" ,"rho", "droppedJobs")
