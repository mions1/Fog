import os
from os import path

def getListFromFile(fileName, z=False):
    means = {"x":[],"y":[],"z":[]}
    with open(fileName,"r") as f:
        for line in f:
            split = line.split()
            means["x"].append(float(split[0]))
            means["y"].append(float(split[1]))
            if z:
                means["z"].append(float(split[2]))
    return means

def joinFiles(dir,files1, files2):

    i=0
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
        for x in x1:
            f.write(str(x)+" "+str(y1[j])+" "+str(y2[j])+"\n")
            j += 1
        f.close()

"""
Parsing FogSF
"""

thr = dict()
responseT = dict()
serviceT = dict()
queueT = dict()
balancerT = dict()
count = dict()
totalDJ = dict()
queueDJ = dict()
balancerDJ = dict()


for file in os.listdir("."):
    if ".sp" in file:
        with open(file) as f:
            key = "Mul_"+file[file.index("mul_")+4:file.index("-rho")]+"-rho_"+file[file.index("rho_")+4:file.index(";")]+".jsp"
            data = getListFromFile(file)
            x1 = data["x"]
            y1 = data["y"]
            x1, y1 = (list(t) for t in zip(*sorted(zip(x1,y1))))
            j = 0
            for x in x1:
                if key not in thr:
                    thr[key] = []
                    responseT[key] = []
                    serviceT[key] = []
                    queueT[key] = []
                    balancerT[key] = []
                    count[key] = []
                    totalDJ[key] = []
                    queueDJ[key] = []
                    balancerDJ[key] = []
                if int(x) not in thr[key]:
                    thr[key].append(int(x))
                if "responseTime" in file:
                    responseT[key].append(y1[j])
                elif "serviceTime" in file:
                    serviceT[key].append(y1[j])
                elif "queuingTime" in file:
                    queueT[key].append(y1[j])
                elif "balancerTime" in file:
                    balancerT[key].append(y1[j])
                elif "Count" in file:
                    count[key].append(y1[j])
                elif "dispatcher" in file:
                    balancerDJ[key].append(y1[j])  
                elif "server[0]" in file:
                    queueDJ[key].append(y1[j])
                elif "droppedJobs" in file:
                    totalDJ[key].append(y1[j])
                                                                              
                j = j+1

for key in thr:
    f = open(key,"w")
    f.write("#thr"+" "+"#responseTime"+" "+"#serviceTime"+" "+"#queuingTime"+" "+"#balancerTime"+" "+"#hop"+" "+"#totalDroppedJobs"+" "+"#dispatcherDroppedJobs"+" "+"#queueDroppedJobs"+"\n")
    for i in range(len(thr[key])):
        f.write(str(thr[key][i])+" "+str(responseT[key][i])+" "+str(serviceT[key][i])+" "+str(queueT[key][i])+" "+str(balancerT[key][i])+" "+str(count[key][i])+" "+str(totalDJ[key][i])+" "+str(balancerDJ[key][i])+" "+str(queueDJ[key][i])+"\n")
    f.close()


"""
Parsing FogQLen

qlen = dict()
qlen2 = dict()
response = dict()
dropped = dict()

for file in os.listdir("."):
    if ".sp" in file:
        with open(file) as f:
            key = "Mul_"+file[file.index("mul_")+4:file.index("-rho")]+"-rho_"+file[file.index("rho_")+4:file.index(";")]+".jsp"
            data = getListFromFile(file)
            x1 = data["x"]
            y1 = data["y"]
            x1, y1 = (list(t) for t in zip(*sorted(zip(x1,y1))))
            j = 0
            for x in x1:
                if key not in qlen:
                    qlen[key] = []
                    qlen2[key] = []
                    response[key] = []
                    dropped[key] = []
                if "response" in file:
                    if float(x) not in qlen[key]:
                        qlen[key].append(float(x))
                        response[key].append(y1[j])
                elif "dropped" in file:
                    if float(x) not in qlen2[key]:
                        qlen2[key].append(float(x)) 
                        dropped[key].append(y1[j])
                j = j+1

for key in qlen:
    f = open(key,"w")
    f.write("#qlen"+" "+"#responseTime"+" "+"#droppedJobs"+"\n")
    print
    for i in range(len(qlen[key])):
        f.write(str(qlen[key][i])+" "+str(response[key][i])+" "+str(dropped[key][i])+"\n")
    f.close()
"""


"""
Parsing FogNoLB

rho = dict()
response = dict()
dropped = dict()

for file in os.listdir("."):
    if ".sp" in file:
        with open(file) as f:
            key = "Mul_"+file[file.index("mul_")+4:file.index("-MG")]+"-MG_"+file[file.index("MG_")+3:file.index(";")]+".jsp"
            data = getListFromFile(file)
            x1 = data["x"]
            y1 = data["y"]
            x1, y1 = (list(t) for t in zip(*sorted(zip(x1,y1))))
            j = 0
            for x in x1:
                if key not in rho:
                    rho[key] = []
                    response[key] = []
                    dropped[key] = []
                if float(x) not in rho[key]:
                    rho[key].append(float(x))
                if "response" in file:
                    response[key].append(y1[j])
                else: 
                    dropped[key].append(y1[j])
                j = j+1

for key in rho:
    f = open(key,"w")
    f.write("#rho"+" "+"#responseTime"+" "+"#droppedJobs"+"\n")
    for i in range(len(rho[key])):
        f.write(str(rho[key][i])+" "+str(response[key][i])+" "+str(dropped[key][i])+"\n")
    f.close()
"""
