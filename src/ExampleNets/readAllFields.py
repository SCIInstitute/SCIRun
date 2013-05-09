import glob
import os
import SCIRunPythonAPI; from SCIRunPythonAPI import *

def allFields(path):
	names = []
	for dirname, dirnames, filenames in os.walk(path):
		for filename in filenames:
			if filename.endswith("fld"):
				names.append(os.path.join(dirname, filename))
	return names
	
dir = r"E:\scirun\trunk_ref\SCIRunData"

for file in allFields(dir):
	read = addModule("ReadField")
	read.Filename = file
	show = addModule("ReportFieldInfo")
	read.output[0] >> show.input.Input
#executeAll()
