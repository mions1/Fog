#!/usr/bin/python
import sys
import os
import re
import math
# usage: meanScalar.py <basefilename> [scalarName, [scalarName, ...]]
# NB: if file are filename-n.sca, n in 1, 2, ... -> basefilename="filename"

class ScalarStat:
	def __init__(self, name="", val=None):
		self.name=name
		self.count=0
		self.sum=0.0
		self.sumsqr=0.0
		self.mean=None
		self.variance=None
		if val!=None:
			self.addValue(val)
	def addValue(self, val):
		if self.mean==None:
			self.count=self.count+1
			self.sum=self.sum+val
		if (self.mean!=None) and (self.variance==None):
			self.sumsqr=self.sumsqr+(val-self.mean)**2
	def __str__(self):
		s="ScalarStat: "+self.name+" (N="+str(self.count)
		if self.mean!=None:
			s=s+" Mean="+str(self.mean)
		if self.variance!=None:
			s=s+" Variance="+str(self.variance)
			if self.variance!=0:
				s=s+" CV="+str(self.variance/self.mean)
		s=s+")"
		return s
	def __unicode__(self):
		return str(self)
	def __repr__(self):
		return str(self)		
	def getMean(self):
		if self.mean==None:
			self.mean=self.sum/self.count
		return self.mean		
	def getVariance(self):
		if self.variance==None:
			self.variance=math.sqrt(self.sumsqr/self.count)
		return self.variance

def usage():
	sys.exit("""  Usage: meanScalar.py <basefilename> [scalarName, [scalarName, ...]]
  NB: if file are filename-n.sca, n in 1, 2, ... -> basefilename="filename" """);

def getFiles(scaBase):
	""" 
	returns a list of files matching the regexp "<scaBase>-[0-9]+.sca"
	"""
	baseDir=os.path.dirname(os.path.abspath(scaBase))
	fileRegexp="^"+os.path.basename(scaBase)+"-[0-9]+\.sca$"
	#print "  baseDir: "+baseDir
	#print "  fileRegexp: "+fileRegexp
	matchedFiles=[baseDir+"/"+f for f in os.listdir(baseDir) if re.match(fileRegexp, f)]
	#print matchedFiles
	return matchedFiles

def getScalarName(s):
	# FIXME: should distinguish name if appProc[N] is present
	#m=re.match("(?P<sname>.+)_[0.9]+$", s)
	#if m:
	#	s=m.group('sname')
	tokens=s.split()
	scaName=tokens[2]
	m=re.search(r'appProc\[(\d+)\]', tokens[1])
	if m:
		scaName=scaName+"_"+m.group(1)
	#print tokens[1] + " / " +tokens[2]
	#print scaName
	return scaName


def getScalars(scaFile):
	""" 
	returns a list of scalars in the .sca file
	"""
	lines=[l for l in open(scaFile) if re.match("^scalar.*", l)]
	# FIXME: should add also component name
	print lines
	scalars=[getScalarName(l) for l in lines]
	return list(set(scalars))

def parseScalars(scaFile, stats):
	for l in open(scaFile):
		if re.match("^scalar.*", l):
			r=l.split()
			s=getScalarName(l)
			v=float(r[3])
			#print "f="+f+" s="+s+" v="+str(v)
			if s in stats:
				stats[s].addValue(v)
			else:
				stats[s]=ScalarStat(name=s, val=v)
	return stats

# build list of .sca files matching the pattern
if len(sys.argv)<2:
	usage()
scaBase=sys.argv[1]
scaFiles=getFiles(scaBase)
if len(scaFiles)<1:
	sys.exit("  Error: no .sca files found!")
# build list of scalar names in the first file
#print scalars
stats={}
# iterate ofer each file
for f in scaFiles:
	# iterate over each scalar
	parseScalars(f, stats)
# for each scalar namecompute average
for s in stats.keys():
	stats[s].getMean()
#compute variance
for f in scaFiles:
	# iterate over each scalar
	parseScalars(f, stats)
for s in stats.keys():
	stats[s].getVariance()
#print everything
if len(sys.argv)>2:
	FilteredScalars=sys.argv[2:]
else:
	FilteredScalars=stats.keys()
for s in FilteredScalars:
	if s in stats:
		stats[s].getVariance()
		print stats[s]
	else:
		print "Scalar \""+s+"\" not found"

