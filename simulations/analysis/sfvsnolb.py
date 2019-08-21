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

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

dir = sys.argv[1]   #Cartella dei files.sca
dir2 = dir if len(sys.argv) <= 2 else sys.argv[2]	#Se non passata, dir2 = dir. Cartella output


#Parse dei file .sca, quindi creazione file .sp (dati in colonna)

scaFiles = sp.getFiles(dir,"sca")   #Recupero files.sca da dir
dictRun = sp.divideByRun(scaFiles)  #Divido i file per le run, utile per fare media
split = dir.split("/")  #splitto per prendere l'ultima parte della directory per fare il nome "1serv20cap" etc, sarà la radice dei file.sp che creerò dopo

#RT e DJ per rho 0.8, thr variabile, mul1, istogrammi

nolbRT = sp.getFiles(dir, "results","in")
seqRT = sp.getFiles(dir, "FogSeq","in")

nolbDJ = sp.getFiles(dir2, "results","in")
seqDJ = sp.getFiles(dir2, "-mul","start")

drawHistOverlayed(dir, nolbRT[0], seqRT, "ResponseTime","ResponseTime","Thr")
drawHistOverlayed(dir2, nolbDJ[0], seqDJ, "DroppedJobs","DroppedJobs","Thr")
