#!/usr/bin env python

# import glob
import os, sys, fileinput
# import time
# import SCIRunPythonAPI; from SCIRunPythonAPI import *
#
# def allFields(path):
# 	names = []
# 	for dirname, dirnames, filenames in os.walk(path):
# 		for filename in filenames:
# 			if filename.endswith("fld"):
# 				names.append(os.path.join(dirname, filename))
# 	return names
#
# def printList(list, name):
# 	thefile = open(name, 'w')
# 	for f,v in list:
# 		thefile.write("%s\n\t%s\n" % (f,v))
#

def makeFileFromTemplate(path, newPath, placeholder, name):
	templatefile = open(path, 'r')
	outputFile = open(newPath, 'w')
	outputFile.writelines([line.replace(placeholder, name) for line in templatefile.readlines()])

def makeModuleHeaderFile(path, name):
	modPath = os.path.join(path, "Modules/Fields/")
	modFile = os.path.join(modPath, name + ".h")
	templatefile = os.path.join(modPath, "../Template/ModuleTemplate.h")
	makeFileFromTemplate(templatefile, modFile, "@ModuleName@", name)
	print("module header: ", modFile)

def makeModuleSourceFile(path, name):
	modPath = os.path.join(path, "Modules/Fields/")
	modFile = os.path.join(modPath, name + ".cc")
	templatefile = os.path.join(modPath, "../Template/ModuleTemplate.cc")
	makeFileFromTemplate(templatefile, modFile, "@ModuleName@", name)
	print("module source: ", modFile)

def makeModuleUnitTestFile(path, name):
	modPath = os.path.join(path, "Modules/Fields/Tests/")
	modFile = os.path.join(modPath, name + "Test.cc")
  templatefile = os.path.join(modPath, "../Template/ModuleUnitTest.cc")
  makeFileFromTemplate(templatefile, modFile, "@ModuleName@", name)
	print("module unit test: ", modFile)

#def editModuleCMake(path, name):
#	print("module cmake edit: ", path, name)

def addModuleToFactory(path, name):
  factorypath = os.path.join(path, "Modules/Factory/ModuleFactoryImpl1.cc") #factoryfile = open(factorypath, 'w')
  replaceLine1 = "//#include <Modules/Fields/@ModuleName@.h>"
  replaceLine2 = "  // insert module desc here"
  for line in fileinput.input(factorypath, inplace=True):
    line = line.replace(replaceLine1, "#include <Modules/Fields/" + name + ".h>\n" + replaceLine1)
    line = line.replace(replaceLine2, "  addModuleDesc<" + name + ">(\"Status\", \"Description\");\n" + replaceLine2)
    sys.stdout.write(line)
  print("module factory edit: ", factorypath, name)

def makeAlgorithmHeaderFile(path, name):
	algoPath = os.path.join(path, "Core/Algorithms/Field/")
	algoFile = os.path.join(algoPath, name + "Algo.h")
	print("algo header: ", algoFile)

def makeAlgorithmSourceFile(path, name):
	algoPath = os.path.join(path, "Core/Algorithms/Field/")
	algoFile = os.path.join(algoPath, name + "Algo.cc")
	print("algo source: ", algoFile)

def makeAlgorithmUnitTestFile(path, name):
	algoPath = os.path.join(path, "Core/Algorithms/Field/Tests/")
	algoFile = os.path.join(algoPath, name + "AlgoTest.cc")
	print("algo unit test: ", algoFile)

def editAlgorithmCMake(path, name):
	print("algorithm cmake edit: ", path, name)

def addAlgorithmToFactory(path, name):
	uiPath = os.path.join(path, "Core/Algorithms/Factory")
	print("algorithm factory edit: ", uiPath, name)

def makeUIDesignerFile(path, name):
	print("ui designer file: ", path, name)

def makeUIHeaderFile(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Fields/")
	print("ui header", uiPath, name)

def makeUISourceFile(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Fields/")
	print("ui source: ", uiPath, name)

def editUICMake(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Fields/")
	print("UI cmake edit: ", uiPath, name)

def addUIToFactory(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Factory/")
	print("UI factory edit: ", uiPath, name)

def makeModuleFiles(path, name):
	makeModuleHeaderFile(path, name)
	makeModuleSourceFile(path, name)
	makeModuleUnitTestFile(path, name)
	#editModuleCMake(path, name)
	addModuleToFactory(path, name)

def makeAlgorithmFiles(path, name):
	makeAlgorithmHeaderFile(path, name)
	makeAlgorithmSourceFile(path, name)
	makeAlgorithmUnitTestFile(path, name)
	editAlgorithmCMake(path, name)
	addAlgorithmToFactory(path, name)

def makeUIFiles(path, name):
	makeUIDesignerFile(path, name)
	makeUIHeaderFile(path, name)
	makeUISourceFile(path, name)
	editUICMake(path, name)
	addUIToFactory(path, name)

try:
	moduleName = sys.argv[1]
except:
	print("Usage:", sys.argv[0], " moduleName [makeUI]"); sys.exit(1)

try:
	makeUI = sys.argv[2]
except:
	makeUI = False
	print("UI flag not specified, not generating UI files")

#print(os.getcwd()
#print(os.path.abspath(os.path.join(os.getcwd(), '../../'))

srcRoot = os.path.abspath(os.path.join(os.getcwd(), '../../'))

#values = []
#files = []

#count = 0

makeModuleFiles(srcRoot, moduleName)
makeAlgorithmFiles(srcRoot, moduleName)
if makeUI:
	makeUIFiles(srcRoot, moduleName)

# these meshes had some issues that crashed scirun, fixed in 5 now:
#r"time-dependent\25feb97_sock_closed" not in file and
#if (r"UCSD\heart-canine" not in file):
#
# for file in allFields(dir):
# 	count += 1
# 	read = addModule("ReadField")
# 	read.Filename = file
# 	files.append(file)
# 	show = addModule("ReportFieldInfo")
# 	prnt = addModule("PrintDatatype")
# 	read.output[0] >> show.input.Input
# 	show.output[0] >> prnt.input[0]
# 	executeAll()
# 	time.sleep(1)
# 	v = prnt.ReceivedValue
# 	values.append(v)
# 	[removeModule(m.id) for m in modules()]
#
#
# printList(zip(files, values), r'E:\fieldTypesAll2.txt')
