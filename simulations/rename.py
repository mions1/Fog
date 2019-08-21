import os
from os import path

def getFiles(dir, ext):
    files = [f for f in os.listdir(dir) if path.isfile(path.join(dir, f)) if f[-len(ext):]==ext]
    return files

def rename(old,new):
    os.rename(old,new)

files = getFiles(".","svg")
for f in files:
    mul = f[f.index("mul_")+4:f.index("-rho")]
    rho = f[f.index("rho_")+4:f.index(".")+2]
    value = f[f.index(".")+2:f.index(".sp")]
    new = value+"-"+"Mul"+mul+"-Rho"+rho+".svg"
    rename(f,new)

