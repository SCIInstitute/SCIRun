import glob
import os
import time
import SCIRunPythonAPI; from SCIRunPythonAPI import *

def allFields(path):
	names = []
	for dirname, dirnames, filenames in os.walk(path):
		for filename in filenames:
			if filename.endswith("fld"):
				names.append(os.path.join(dirname, filename))
	return names
	
def printList(list, name):
	thefile = open(name, 'w')
	for f,v in list:
		thefile.write("%s\n\t%s\n" % (f,v))
	

dir = r"E:\scirun\trunk_ref\SCIRunData"

values = []
files = []

count = 0

# these meshes had some issues that crashed scirun, fixed in 5 now:
#r"time-dependent\25feb97_sock_closed" not in file and 
#if (r"UCSD\heart-canine" not in file):
read = addModule("ReadField")
show = addModule("ReportFieldInfo")
prnt = addModule("PrintDatatype")
read.output[0] >> show.input.InputField
show.output[0] >> prnt.input[0]

for file in allFields(dir):
	count += 1
	read.Filename = file
	files.append(file)
	executeAll()
	v = prnt.ReceivedValue
	values.append(v)

printList(zip(files, values), r'E:\fieldTypesAll2.txt')