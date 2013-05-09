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

for file in allFields(dir):
	read = addModule("ReadField")
	read.Filename = file
	files.append(file)
	show = addModule("ReportFieldInfo")
	prnt = addModule("PrintDatatype")
	read.output[0] >> show.input.Input
	show.output[0] >> prnt.input[0]
	executeAll()
	time.sleep(1)
	values.append(prnt.ReceivedValue)
	[removeModule(m.id) for m in modules()]

printList(zip(files, values), r'E:\fieldTypes.txt')