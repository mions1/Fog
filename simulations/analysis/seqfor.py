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

#---------------------------------------Analisi SequentialForward-----------------------

def makeDroppedJobsSeq(dir, dir2, dictRun, type=["doppedJobsSLA"], type2="internalJobs", where="dispatcher", preset=3, much=1):
    #Sostanzialmente fa quello che fa makeFilesNoLB ma in più deve normalizzare i valori di droppedJob
    values1=sp.getValue(dictRun, dir, type, where,much)
    values2=sp.getValue(dictRun, dir, type2, where) #nInternalJobs
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
    sp.makeFile(means, path.join(dir2), where+type[0]+"frattoTotalJobs", preset=preset)
    files = sp.getFiles(dir2, "frattoTotalJobs.sp")
    #drawSingle(dir2, files)
    #drawDouble(dir2, together(files,"_001"),"_001;droppedJobs")
    #drawDouble(dir2, together(files,"_1"),"_1;droppedJobs")
    return files

def makeFiles1VSFiles2SF(files1, files2, dir2, valuesFile1, valuesFile2, xlabel=""):
    files1.sort()
    files2.sort()
    #unisce i file di cui vogliamo fare i grafici uniti
    #creando un file a 3 colonne (rho(x) - values1(y1) - values2(y2))
    sp.joinFiles(dir2,files1,files2)

    #Recupero i files appena creati
    filesJoin = sp.getFiles(dir2, valuesFile2+"_1_joined.sp")

    #Plot dei grafici uniti
    #thr = [i+1 for i in range(20)]
    if xlabel == "thr":
        rho = ["0.2","0.5","0.8","0.9"]
        for i in rho:
            p.drawOverlayed(dir2, together(filesJoin,"mul_10-rho_"+str(i)+";","joined.sp"), "_"+str(i), valuesFile1, valuesFile2, namex=xlabel)
            p.drawOverlayed(dir2, together(filesJoin,"mul_001-rho_"+str(i)+";","joined.sp"), "_"+str(i), valuesFile1, valuesFile2, namex=xlabel)
        #drawOverlayed(dir2, together(filesJoin,"mul_001","joined.sp"), "_001", valuesFile1, valuesFile2)
    elif xlabel == "rho":
        thr = [i+1 for i in range(20)]
        for i in thr:
            p.drawOverlayed(dir2, together(filesJoin,"mul_10-thr_"+str(i)+";","joined.sp"), "_"+str(i), valuesFile1, valuesFile2, namex=xlabel)
            p.drawOverlayed(dir2, together(filesJoin,"mul_001-thr_"+str(i)+";","joined.sp"), "_"+str(i), valuesFile1, valuesFile2, namex=xlabel)
        #drawOverlayed(dir2, together(filesJoin,"mul_001","joined.sp"), "_001", valuesFile1, valuesFile2)
    ansca.deleteFiles(filesJoin, dir2)

def drawMore(dir, files, name, x_label="", y_label="", start="", end="", preset="bdTime"):
    for file in files:
        data = sp.getListFromFile(path.join(dir, file))
        data["x"], data["y"] = (list(t) for t in zip(*sorted(zip(data["x"], data["y"]))))
        if preset=="bdTime":
            if "service" in file:
                plt.plot(data["x"], data["y"], 'r--', label="serviceTime")
            if "queuing" in file:
                plt.plot(data["x"], data["y"], 'b--', label="queuingTime")
            if "balancer" in file:
                plt.plot(data["x"], data["y"], 'g--', label="balancerTime")  # , t, t ** 2, 'bs', t, t ** 3, 'g^')
            if "response" in file:
                plt.plot(data["x"], data["y"], 'k--', label="responseTime")
            
            """
            if "mul_001" in file:
                plt.ylim(0.0, 0.16)
            elif "mul_10" in file:
                plt.ylim(0.0, 16.0)
            """
        
        if preset=="bdJobs":
            if "server[0]" in file:
                plt.plot(data["x"], data["y"], 'r--', label="droppedServerJobs")
            elif "dispatcher" in file:
                plt.plot(data["x"], data["y"], 'g--', label="droppedDispatcherJobs")
            elif "SLA" in file:
                plt.plot(data["x"], data["y"], 'b--', label="droppedJobsTotal")

            plt.ylim(0.0, 1.0)
                
        elif preset=="qlen":
            if "rho_0.2" in file:
                plt.plot(data["x"], data["y"], 'r--', label="rho 0.2")
            if "rho_0.5" in file:
                plt.plot(data["x"], data["y"], 'b--', label="rho 0.5")
            if "rho_0.8" in file:
                plt.plot(data["x"], data["y"], 'g--', label="rho 0.8")
            if "rho_0.9" in file:
                plt.plot(data["x"], data["y"], 'k--', label="rho 0.9")

    #plt.title(name)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.savefig(path.join(dir, *["plot", name + ".svg"]))
    plt.clf()
    pass

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

#Terze Analisi - FogSequentialForward

#cartella rho

#makeFile(dir, dir2, dictRun, "rho", preset=3)
#makeFile(dir, dir2, dictRun, "droppedJobsSLA_class_1", preset=3)
ansca.makeFile(dir, dir2, dictRun, "avg_responseTime_1", preset=3)
ansca.makeFile(dir, dir2, dictRun, "avg_balancerCount_1", preset=3)
ansca.makeFile(dir, dir2, dictRun, "avg_balancerTime_1", preset=3)
ansca.makeFile(dir, dir2, dictRun, "avg_serviceTime_1", preset=3)
ansca.makeFile(dir, dir2, dictRun, "avg_queuingTime_1", preset=3)

#cartella thr

#makeFile(dir, dir2, dictRun, "rho", preset=4)
#makeFile(dir, dir2, dictRun, "droppedJobsSLA_class_1", preset=4)
ansca.makeFile(dir, dir2, dictRun, "avg_responseTime_1", preset=4)
ansca.makeFile(dir, dir2, dictRun, "avg_balancerCount_1", preset=4)
ansca.makeFile(dir, dir2, dictRun, "avg_balancerTime_1", preset=4)
ansca.makeFile(dir, dir2, dictRun, "avg_serviceTime_1", preset=4)
ansca.makeFile(dir, dir2, dictRun, "avg_queuingTime_1", preset=4)

#Creo plot dei precedenti file
filesRT = sp.getFiles(dir2, "responseTime_1.sp")
#drawSingle(dir2, files, x_label="thr", y_label="responseTime")

#Creo plot dei precedenti file
filesRho = sp.getFiles(dir2, "rho.sp")
#drawSingle(dir2, files, x_label="thr", y_label="rho")

filesBC = sp.getFiles(dir2, "balancerCount_1.sp")
#drawSingle(dir2, files, x_label="thr", y_label="avg_balancerCount_1")

filesST = sp.getFiles(dir2, "serviceTime_1.sp")
#drawSingle(dir2, files, x_label="thr", y_label="avg_serviceTime_1")

filesQT = sp.getFiles(dir2, "queuingTime_1.sp")
#drawSingle(dir2, files, x_label="thr", y_label="avg_queuingTime_1")

filesBT = sp.getFiles(dir2, "balancerTime_1.sp")
#drawSingle(dir2, files, x_label="thr", y_label="avg_balancerTime_1")


makeDroppedJobsSeq(dir, dir2, dictRun, ["droppedJobsSLA"],"internalJobs","",4,much=2)
filesDJ1 = sp.getFiles(dir2, "SLAfrattoTotalJobs.sp")

makeDroppedJobsSeq(dir, dir2, dictRun, ["droppedJobsSLA"],"internalJobs","dispatcher",4,much=1)
filesDJ2 = sp.getFiles(dir2, "QueuefrattoTotalJobs.sp")

makeDroppedJobsSeq(dir, dir2, dictRun, ["droppedJobsSLA"],"totalJobs","server[0]",4,much=1)
filesDJ3 = sp.getFiles(dir2, "QueuefrattoTotalJobs.sp")
#drawSingle(dir2, files, x_label="thr", y_label="droppedJobsSLA/totalJobs")

#makeFiles1VSFiles2(filesRT, filesDJ, "responseTime", "% droppedJobs")

#VARIA THR - cartella rho
#makeFiles1VSFiles2SF(filesDJ1, filesRT, dir2, "droppedJobs", "responseTime", "thr")
makeFiles1VSFiles2SF(filesBC, filesBT, dir2, "Hop", "balancerTime", "thr")

filesTime = sp.getFiles(dir2, "Time_1.sp")
filesJobs = sp.getFiles(dir2, "TotalJobs.sp")
rho = ["0.2","0.5", "0.9"]
mul = ["001","10"]

#responseTime
for r in rho:
    for m in mul:
        drawMore(dir2, sp.getFilesFromList(filesTime,"mul_"+m+"-rho_"+r,"in"),"_"+"mul_"+m+"-rho_"+r+";Time",x_label="thr",y_label="*Time",start="",end="")

#droppedJobs
for r in rho:
    for m in mul:
        drawMore(dir2, sp.getFilesFromList(filesJobs,"mul_"+m+"-rho_"+r,"in"),"_"+"mul_"+m+"-rho_"+r+";droppedJobs",x_label="thr",y_label="droppedJobs",start="",end="", preset="bdJobs")


#VARIA RHO - cartella thr
#makeFiles1VSFiles2SF(filesDJ1, filesRT, dir2, "droppedJobs", "responseTime", "rho")
#makeFiles1VSFiles2SF(filesBC, filesBT, dir2, "Hop", "balancerTime", "rho")

filesTime = sp.getFiles(dir2, "Time_1.sp")
filesJobs = sp.getFiles(dir2, "TotalJobs.sp")
thr = [5,15]
mul = ["001","10"]

#responseTime
for t in thr:
    for m in mul:
        p.drawMore(dir2, sp.getFilesFromList(filesTime,"mul_"+m+"-thr_"+str(t)+";drop","in"),"_"+"mul_"+m+"-thr_"+str(t)+";Time",x_label="rho",y_label="*Time",start="",end="")
     
#droppedJobs
for t in thr:
    for m in mul:
        p.drawMore(dir2, sp.getFilesFromList(filesJobs,"mul_"+m+"-thr_"+str(t)+";","in"),"_"+"mul_"+m+"-thr_"+str(t)+";droppedJobs",x_label="rho",y_label="droppedJobs",start="",end="", preset="bdJobs")

