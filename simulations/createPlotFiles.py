#!/usr/bin/python
import sys
import os
import subprocess
import tempfile
import re

def usage():
	sys.exit("  Usage: createPlotFiles.py <filename.vec>");

def initFile(f):
	try:
		os.remove(f)
	except OSError:
		pass

def isNumber(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

if len(sys.argv)<2:
	usage()

inFile=sys.argv[1]

listOut=subprocess.check_output(["scavetool", "list", inFile])
#print listOut
# match the lines in the forms serviceTime_[0-9]+
vectors=[v for v in listOut.split() if re.match("responseTime_[0-9]+", v)]
#print vectors
tmpFile="/tmp/"+str(os.getpid())+".tmp"
# iterate over the vectors of interest
for v in vectors:
	print "processing vector "+v
	# initialize tmp file
	initFile(tmpFile)
	# print header in CSVFile
	CSVFile=v+".csv"
	initFile(CSVFile)
	CSVHandle = open(CSVFile,"w")
	CSVHandle.write("#timeStamp responseTime\n")
	# create tmp file
	subprocess.call(["scavetool", "vector", "-p", "name("+v+")", "-F", "csv", "-O", tmpFile, inFile])
	# parse tmp file
	with open(tmpFile, "rt") as vecHandle:		
		# skip first row
		#print (vecHandle.read())
		# iterate over file
		for l in vecHandle:
			l=l.strip()
			#print "\""+l+"\""
			fields=l.split(',')
			if isNumber(fields[0]) and isNumber(fields[1]):
				ts=float(fields[0])
				val=float(fields[1])
				CSVHandle.write(str(round(ts-val,5))+" "+str(val)+"\n")
		# end with
	CSVHandle.close()
# cleanup
initFile(tmpFile)

