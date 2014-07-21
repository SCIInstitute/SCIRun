#!/usr/bin env python

import os, sys, fileinput

def makeFileFromTemplate(path, newPath, placeholder, name):
	templatefile = open(path, 'r')
	outputFile = open(newPath, 'w')
	outputFile.writelines([line.replace(placeholder, name) for line in templatefile.readlines()])

def makeModuleFile(rootpath, name, filepath, extension, templatefilepath):
	modPath = os.path.join(rootpath, filepath)
	modFile = os.path.join(modPath, name + extension)
	templatefile = os.path.join(modPath, templatefilepath)
	makeFileFromTemplate(templatefile, modFile, "@ModuleName@", name)
	print("module file: ", modFile)

def makeModuleHeaderFile(path, name):
	makeModuleFile(path, name, "Modules/Fields/", ".h", "../Template/ModuleTemplate.h")

def makeModuleSourceFile(path, name):
	makeModuleFile(path, name, "Modules/Fields/", ".cc", "../Template/ModuleTemplate.cc")

def makeModuleUnitTestFile(path, name):
	makeModuleFile(path, name, "Modules/Fields/Tests", "Tests.cc", "../../Template/ModuleUnitTest.cc")

def editCMake(file, name, srcLine, headerLine, doHeader):
  for line in fileinput.input(file, inplace=True):
    line = line.replace(srcLine, srcLine + "\n  " + name + ".cc")
    if doHeader:
      line = line.replace(headerLine, headerLine + "\n  " + name + ".h")
    sys.stdout.write(line)
  print("cmake edit: ", file, name)

def editModuleCMake(path, name):
  modPath = os.path.join(path, "Modules/Fields/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Modules_Fields_SRCS"
  replaceLineHeader = "SET(Modules_Fields_HEADERS"
  editCMake(cmakeFile, name, replaceLineSrc, replaceLineHeader, True)

def editModuleTestCMake(path, name):
  modPath = os.path.join(path, "Modules/Fields/Tests/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Modules_Fields_Tests_SRCS"
  editCMake(cmakeFile, name + "Tests", replaceLineSrc, "", False)

def addModuleToFactory(path, name):
  factorypath = os.path.join(path, "Modules/Factory/ModuleFactoryImpl1.cc") #factoryfile = open(factorypath, 'w')
  replaceLine1 = "//#include <Modules/Fields/@ModuleName@.h>"
  replaceLine2 = "  // insert module desc here"
  for line in fileinput.input(factorypath, inplace=True):
    line = line.replace(replaceLine1, "#include <Modules/Fields/" + name + ".h>\n" + replaceLine1)
    line = line.replace(replaceLine2, "  addModuleDesc<" + name + ">(\"Status\", \"Description\");\n" + replaceLine2)
    sys.stdout.write(line)
  print("module factory edit: ", factorypath, name)

def makeAlgorithmFile(rootpath, name, filepath, extension, templatefilepath):
	algoPath = os.path.join(rootpath, filepath)
	algoFile = os.path.join(algoPath, name + extension)
	templatefile = os.path.join(algoPath, templatefilepath)
	makeFileFromTemplate(templatefile, algoFile, "@AlgorithmName@", name)
	print("algorithm file: ", algoFile)

def makeAlgorithmHeaderFile(path, name):
	makeAlgorithmFile(path, name, "Core/Algorithms/Field/", "Algo.h", "../Template/AlgorithmTemplate.h")

def makeAlgorithmSourceFile(path, name):
	makeAlgorithmFile(path, name, "Core/Algorithms/Field/", "Algo.cc", "../Template/AlgorithmTemplate.cc")

def makeAlgorithmUnitTestFile(path, name):
	makeAlgorithmFile(path, name, "Core/Algorithms/Field/Tests/", "AlgoTests.cc", "../../Template/AlgorithmTestTemplate.cc")

def editAlgorithmCMake(path, name):
	modPath = os.path.join(path, "Core/Algorithms/Field/")
	cmakeFile = os.path.join(modPath, "CMakeLists.txt")
	replaceLineSrc = "SET(Algorithms_Field_SRCS"
	replaceLineHeader = "SET(Algorithms_Field_HEADERS"
	editCMake(cmakeFile, name, replaceLineSrc, replaceLineHeader, True)
	print("algorithm cmake edit: ", path, name)

def editAlgorithmTestCMake(path, name):
  modPath = os.path.join(path, "Core/Algorithms/Field/Tests/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Algorithms_Field_Tests_SRCS"
  editCMake(cmakeFile, name + "Tests", replaceLineSrc, "", False)

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
	editModuleCMake(path, name)
	editModuleTestCMake(path, name)
	addModuleToFactory(path, name)

def makeAlgorithmFiles(path, name):
	makeAlgorithmHeaderFile(path, name)
	makeAlgorithmSourceFile(path, name)
	makeAlgorithmUnitTestFile(path, name)
	editAlgorithmCMake(path, name)
	#editAlgorithmTestCMake(path, name)
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
	print("Usage:", sys.argv[0], " moduleName [--algo] [--ui]"); sys.exit(1)

restofargs = sys.argv[2:]
makeAlgo = "--algo" in restofargs
makeUI = "--ui" in restofargs

if not makeUI:
	print("UI flag not specified, not generating UI files")
if not makeAlgo:
	print("Algo flag not specified, not generating algorithm files")

#print(os.getcwd()
#print(os.path.abspath(os.path.join(os.getcwd(), '../../'))

srcRoot = os.path.abspath(os.path.join(os.getcwd(), '../../'))

#values = []
#files = []

#count = 0

makeModuleFiles(srcRoot, moduleName)
if makeAlgo:
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
