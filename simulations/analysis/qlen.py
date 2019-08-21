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

"""
#Seconde analisi - QLEN
"""

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

#Creo file di rho/responseTime/dropped jobs in relazione alla q-len

makeFile(dir, dir2, dictRun, "rho", preset=2)
makeFile(dir, dir2, dictRun, "avg_responseTime_1", preset=2)
filesDJ = ansca.makeDroppedJobs(dir, dir2, dictRun, preset=2)

#Creo plot dei precedenti file
filesRho = sp.getFiles(dir2, "rho.sp")
p.drawSingle(dir2, files, x_label="q_len", y_label="rho")

#Creo plot dei precedenti file
filesRT = sp.getFiles(dir2, "responseTime_1.sp")
p.drawSingle(dir2, files, x_label="q_len", y_label="responseTime")

#Creo plot dei precedenti file
filesDJ = sp.getFiles(dir2, "droppedJobs.sp")
p.drawSingle(dir2, files, x_label="q_len", y_label="droppedJobs")

mul = ["001","1"]
for m in mul:
    p.drawMore(dir2,sp.getFilesFromList(filesRT, "mul_"+m,"in"), "_"+"mul_"+m+";responseTime", x_label="capacità", y_label="responseTime", start="", end="", preset="qlen")
    p.drawMore(dir2,sp.getFilesFromList(filesDJ, "mul_"+m,"in"), "_"+"mul_"+m+";droppedJobs", x_label="capacità", y_label="% droppedJobs", start="", end="", preset="qlen")
