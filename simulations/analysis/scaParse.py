from os import path
import os
import statistics
import sys, glob

"""
	Parsing dei file.sca
	Usato per creare file.sp che contengono, in due colonne, i valori di rho ed i valori, ad es., del response time
	crea un file.sp facendo la media dei valori di ogni run
"""

"""
Prende i valori name dai files.sca che si trovano in dirpath
creando un dizionario dove le key sono i file che creeremo (quindi le cat. dei run)
e i valori sono i dati

*Files -> dizionario, le key sono le cat. dei run
        -> mentre gli elementi sono liste di ogni run, create con divideByRun per intenderci
*dirPath -> directory dove cercare i file
*name -> lista dei valori che si vogliono prendere
*much -> quanti di quei valori per ogni key, ad es. se voglio il totale dei droppedJobs tra server e dispatcher
"""
def getValue(files, dirPath, name=[], where="", much=1):
    values = dict()
    i = 1
    somma = 0.0
    for key in files:
        for value in files[key]:
            fileName = path.join(dirPath,value)
            with open(fileName, 'r') as f:
                for line in f:
                    if "scalar" in line:
                        tmp = line.split()
                        if tmp[-2] in name:
                            if str(where) in tmp[-3]:
                                if key not in values:
                                    values[key] = list()
                                if i < much:
                                    somma += float(tmp[-1])
                                    i += 1
                                else:
                                    values[key].append(float(tmp[-1])+somma)
                                    somma = 0.0
                                    i = 1
    return values

"""
Restituisce la lista dei file in dir che finiscono (iniziano)(sono in mezzo) con pattern
*where può essere: end, start, in
"""
def getFiles(dir,pattern, where="end"):
    if where=="start":
        files = [f for f in os.listdir(dir) if path.isfile(path.join(dir, f)) if f[:len(pattern)]==pattern]
    elif where=="end":
        files = [f for f in os.listdir(dir) if path.isfile(path.join(dir, f)) if f[-len(pattern):]==pattern]
    elif where=="in":
        files = [f for f in os.listdir(dir) if path.isfile(path.join(dir, f)) if pattern in f]
    return files
    
"""
Come getFiles, ma invece che prendere la lista di file dalla directory, li prende dalla lista passata
"""
def getFilesFromList(lista,pattern, where="end"):
    if where=="start":
        files = [f for f in lista if f[:len(pattern)]==pattern]
    elif where=="end":
        files = [f for f in lista if f[-len(pattern):]==pattern]
    elif where=="in":
        files = [f for f in lista if pattern in f]
    return files

"""
Crea un nuovo file che ha 3 colonne:
x -> X in comune
y1-> Y del file1
y2-> Y del file2
Messe in ordine per X
"""
def joinFiles(dir,files1, files2, header=False, x_header="", y1_header="", y2_header="", header_div="#", div=" "):

    for i in range(len(files1)):
        file1 = files1[i]
        file2 = files2[i]
        data1 = getListFromFile(path.join(dir,file1))
        data2 = getListFromFile(path.join(dir, file2))
        x1 = data1["x"]
        y1 = data1["y"]
        x2 = data2["x"]
        y2 = data2["y"]
        x1, y1 = (list(t) for t in zip(*sorted(zip(x1,y1))))
        x2, y2 = (list(t) for t in zip(*sorted(zip(x2,y2))))
        f = open(path.join(dir,file1[:file1.index(".sp")]+"VS"+file2[:file2.index(".sp")]+"_joined.sp"), "w")
        j = 0
        if header:
            f.write(header_div+x_header +div+ header_div+y1_header +div+ header_div+y2_header+"\n")
        for x in x1:
            f.write(str(x) +div+ str(y1[j]) +div+ str(y2[j])+"\n")
            j += 1
        f.close()

"""
Divide la lista dei file, creata con getFiles, creando un dizionario in cui
le key sono la categoria(nome configurazione senza #0-..-#n) ed i valori sono
i singoli file delle run
"""
def divideByRun(scaFiles):
    files = dict()
    for f in scaFiles:
        if f[:f.index("#")-1] not in files:
            files[f[:f.index("#")-1]] = list()
        files[f[:f.index("#")-1]].append(f)
    return files

"""
Restituisce la media dei valori per ogni key nel dizionario.
Usata per prendere la media dei valori di ogni run in seguito a getValue
"""
def getMeans(values):
    means = dict()
    for key in values:
        means[key] = statistics.mean(values[key])
    return means

"""
Restituisce i valori che si trovano nel file fileName(.sp)
come una lista di 2 (o 3, nel caso siano fatti con join) colonne, x-y-z
"""
def getListFromFile(fileName, z=False, header="#"):
    means = {"x":[],"y":[],"z":[]}
    with open(fileName,"r") as f:
        for line in f:
            if header not in line:
                split = line.split()
                means["x"].append(float(split[0]))
                means["y"].append(float(split[1]))
                if z:
                    means["z"].append(float(split[2]))
    return means

"""
	Usato per creare files.sp che contengono, in due colonne, per ogni valore di, ad es., rho, la media di, ad es., responseTime.
	Per ogni run crea un file.sp unendo tutte le run di una stessa configurazione facendone la media
"""
def makeFile(means, fileName, fileSuffix="", preset=1, header=False):
    """
    fileList = glob.glob(fileName+"*"+fileSuffix+"*.sp",recursive=False)
    for file in fileList:
        try:
            os.remove(file)
        except OSError:
            print("Error while deleting")
    """
    
    values = dict()
    i = 0
    #Usato per configurazioni a cui cambiano mul e rho
    if preset==1:
        for key in means:
            if "Base" not in key:
                rho = key[key.index("rho_")+4:key.index("_MG")]
                rho = float(rho[0]+"."+rho[1:])
                mul = key[key.index("_")+1:key.index("-rho")]
                MG = key[key.index("MG")+2:]
                mean = means[key]
                file = fileName+"-mul_"+str(mul)+"-MG_"+MG+";"+fileSuffix+".sp"
                f = open(fileName+"-mul_"+str(mul)+"-MG_"+MG+";"+fileSuffix+".sp","a")
                if file not in values:
                    values[file] = []
                values[file].append(rho)
                values[file].append(mean)

    #Usato per configurazioni a cui cambiano mul, rho e thr
    elif preset==3:
        for key in means:
            if "Base" not in key:
                rho = key[key.index("rho_")+4:key.index("-thr")]
                rho = float(rho[0]+"."+rho[1:])
                mul = key[key.index("_")+1:key.index("-rho")]
                thr = key[key.index("thr_")+4:]
                mean = means[key]
                f = open(fileName+"-mul_"+str(mul)+"-rho_"+str(rho)+";"+fileSuffix+".sp","a")
                if i == 0 and header:
                    f.write("#thr"+" "+"#"+fileSuffix+"\n")
                    i += 1
                f.write(str(thr)+" "+str(mean)+"\n")
                f.close()
                
    #Usato come per quello di prima, ma al variare di rho
    elif preset==4:
        for key in means:
            if "Base" not in key:
                rho = key[key.index("rho_")+4:key.index("-thr")]
                rho = float(rho[0]+"."+rho[1:])
                mul = key[key.index("_")+1:key.index("-rho")]
                thr = key[key.index("thr_")+4:]
                mean = means[key]
                f = open(fileName+"-mul_"+str(mul)+"-thr_"+str(thr)+";"+fileSuffix+".sp","a")
                if i == 0 and header:
                    f.write("#rho"+" "+"#"+fileSuffix+"\n")
                    i += 1
                f.write(str(rho)+" "+str(mean)+"\n")
                f.close()
                   
	#Usato per configurazioni a cui cambia la q-len
    elif preset==2:
        for key in means:
            if "Base" not in key:
                q_len = int(key[key.index("thr_")+4:])
                rho = key[key.index("rho_")+4:key.index("-thr")]
                rho = float(rho[0]+"."+rho[1:])
                mul = key[key.index("_")+1:key.index("-rho")]
                mean = means[key]
                f = open(fileName+"-mul_"+str(mul)+"-rho_"+str(rho)+";"+fileSuffix+".sp","a")
                if i == 0 and header:
                    f.write("#q_len"+" "+"#"+fileSuffix+"\n")
                    i += 1
                f.write(str(q_len)+" "+str(mean)+"\n")
                f.close()

if __name__=="__main__":
    dir = sys.argv[1]
    fileBase = sys.argv[2]
    valueName = sys.argv[3]
     
    #print("Dir: "+str(dir))
    scaFiles = getSca(dir)
    #print("sca files: "+str(scaFiles))
    dictRun = divideByRun(scaFiles)
    """
    #print("dict run: "+str(dictRun))
    values = getValue(dictRun, dir,valueName)
    #print ("Values: "+str(values))
    means = getMeans(values)
    #print(means)
    makeFile(means, fileBase)
    """  
    
