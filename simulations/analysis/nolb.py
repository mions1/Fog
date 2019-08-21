import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import plot as p
from scipy.stats import norm
from scipy.stats import lognorm
from scipy.stats import gamma
from scipy.optimize import curve_fit
import analysis_sca as ansca
import vectorParse as vp
import scaParse as sp
import sys, glob, os
from os import path

#-----------------Analisi FogNoLB----------------------------
def makeFilesNoLB(dir, dir2, dictRun, valuesName):
    #Creo files.sp per valuesName
    #cioè un file che ha due colonne, la prima con rho e la seconda con i valori delle medie di valuesName
    ansca.makeFile(dir, dir2, dictRun, valuesName)
    #recupero i file appena creati
    files = sp.getFiles(dir2, valuesName+".sp")
    #disegno il singolo plot (es. per 1serv20cap)
    #drawSingle(dir2, files, "rho",valuesName)
    #disegno i plot uniti (stesso mul, per 1serv e 20serv)
    #drawDouble(dir2, together(files,"mul_001"),"_001;"+valuesName,x_label="rho",y_label=valuesName,start="MG",end=";")
    #drawDouble(dir2, together(files,"mul_1"),"_1;"+valuesName,x_label="rho",y_label=valuesName,start="MG",end=";")
    return files

def makeDroppedJobs(dir, dir2, dictRun, preset=1):
    #Sostanzialmente fa quello che fa makeFilesNoLB ma in più deve normalizzare i valori di droppedJobs
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
    
#---------------------------------------------

"""
Prime analisi - FogNoLB
"""

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

filesResponseTime = makeFilesNoLB(dir, dir2, dictRun, "responseTime")
filesRho = makeFilesNoLB(dir, dir2, dictRun, "rho")
filesDroppedJobs = makeDroppedJobs(dir, dir2, dictRun)
filesBalancerTime = makeFilesNoLB(dir, dir2, dictRun, "avg_balancerTime_1")


makeFiles1VSFiles2(filesResponseTime, filesRho, dir2, "responseTime", "rho", True)
ansca.makeFiles1VSFiles2(filesDroppedJobs, filesResponseTime, dir2, "droppedJobs", "responseTime", True)
makeFiles1VSFiles2(filesDroppedJobs, filesRho, dir2, "droppedJobs", "rho", True)

#Elimino file temporanei
#cleanDir(path.join(dir2, "*joined*.sp"))
