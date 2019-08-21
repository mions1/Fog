from os import path
import os
import statistics
import sys

"""
	Parsing dei file.vec
	Usato per creare file.vp che contengono, in una colonna, tutti i valori, ad es., del response time
	crea un file.vp unendo tutte le run di una stessa configurazione
"""

"""
Prende i valori name dai files.vec che si trovano in dirpath
creando un dizionario dove le key sono i file che creeremo (quindi le cat. dei run)
e i valori sono i dati

*Files -> dizionario, le key sono le cat. dei run
        -> mentre gli elementi sono liste di ogni run
*dirPath -> directory dove cercare i file
"""
def getValues(files, dirPath, name="responseTime"):
    values = dict()
    for key in files:
        for value in files[key]:
            fileName = path.join(dirPath,value)
            index = -1
            with open(fileName, 'r') as f:
                for line in f:
                    if index == -1:
                        split = line.split()
                        if len(split)>0 and "vector" in split[0]:
                            if name in split[3]:
                                index = split[1]
                    else:
                        split = line.split("\t")
                        if index in split[0]:
                            if key not in values:
                                values[key] = list()
                            values[key].append(split[-1][:-2])
    return values

"""
	NON USATA
"""
def valuesToString(values):
    text = ""
    for value in values:
        text += str(value)+" "
    text = text[:-1]
    return text

"""
Restituisce la lista dei file con estensione .ext che si trovano in dir
"""
def getFiles(dir,ext):
    files = [f for f in os.listdir(dir) if path.isfile(path.join(dir, f)) if f[-len(ext):]==ext]
    return files

"""
Divide la lista dei file, creata con getFiles, creando un dizionario in cui
le key sono la categoria(nome configurazione senza #0-..-#n)
i valori sono i dati cercati (es. response time)
"""
def divideByRun(vecFiles):
    files = dict()
    for f in vecFiles:
        if f[:f.index("#")-1] not in files:
            files[f[:f.index("#")-1]] = list()
        files[f[:f.index("#")-1]].append(f)
    return files

"""
Restituisce la media dei valori per ogni key nel dizionario (NON USATA)
"""
def getMeans(values):
    means = dict()
    for key in values:
        means[key] = statistics.mean(values[key])
    return means

"""
	Usato per creare file.vp che contengono, in una colonna, tutti i valori, ad es., del response time
	crea un file.vp unendo tutte le run di una stessa configurazione
"""
def makeFile(values, fileName):
    for key in values:
        if "Base" not in key:
            f = open(fileName + key +".vp", "a")
            for value in values[key]:
                f.write(str(value)+"\n")
            f.close()

if __name__ == "__main__":
    dir = sys.argv[1]		#Directory dei file .vec da parsare
    fileBase = sys.argv[2]	#Percorso dove salvare i file .vp creati con makeFile
    valueName = sys.argv[3]	#Nome dei valori da prendere, es responseTime

    vecFiles = getFiles(dir,"vec")	#Prendo tutti i file .vec in dir

    dictRun = divideByRun(vecFiles)	#Divido i file per run

    values = getValues(dictRun, dir, valueName)	#Prendo i valori, ad es del responseTime

    makeFile(values, fileBase)	#Creo il file uni-colonnare dei valori
