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

def editCMake(file, name, srcLine, headerLine = "", formLine = ""):
  for line in fileinput.input(file, inplace=True):
    if len(srcLine) > 0:
		  line = line.replace(srcLine, srcLine + "\n  " + name + ".cc")
    if len(headerLine) > 0:
      line = line.replace(headerLine, headerLine + "\n  " + name + ".h")
    if len(formLine) > 0:
      line = line.replace(formLine, formLine + "\n  " + name + ".ui")
    sys.stdout.write(line)
  print("cmake edit: ", file, name)

def editModuleCMake(path, name):
  modPath = os.path.join(path, "Modules/Fields/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Modules_Fields_SRCS"
  replaceLineHeader = "SET(Modules_Fields_HEADERS"
  editCMake(cmakeFile, name, replaceLineSrc, replaceLineHeader)

def editModuleTestCMake(path, name):
  modPath = os.path.join(path, "Modules/Fields/Tests/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Modules_Fields_Tests_SRCS"
  editCMake(cmakeFile, name + "Tests", replaceLineSrc)

def addModuleToFactory(path, name):
  factorypath = os.path.join(path, "Modules/Factory/ModuleFactoryImpl1.cc")
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
	algoPath = os.path.join(path, "Core/Algorithms/Field/")
	cmakeFile = os.path.join(algoPath, "CMakeLists.txt")
	replaceLineSrc = "SET(Algorithms_Field_SRCS"
	replaceLineHeader = "SET(Algorithms_Field_HEADERS"
	editCMake(cmakeFile, name + "Algo", replaceLineSrc, replaceLineHeader)
	print("algorithm cmake edit: ", path, name)

def editAlgorithmTestCMake(path, name):
  modPath = os.path.join(path, "Core/Algorithms/Field/Tests/")
  cmakeFile = os.path.join(modPath, "CMakeLists.txt")
  replaceLineSrc = "SET(Algorithms_Field_Tests_SRCS"
  editCMake(cmakeFile, name + "AlgoTests", replaceLineSrc)

def addAlgorithmToFactory(path, name):
	factorypath = os.path.join(path, "Core/Algorithms/Factory/HardCodedAlgorithmFactory.cc")
	replaceLine1 = "#include <boost/functional/factory.hpp>"
	replaceLine2 = "    ;"
	for line in fileinput.input(factorypath, inplace=True):
		line = line.replace(replaceLine1, "#include <Core/Algorithms/Field/" + name + "Algo.h>\n" + replaceLine1)
		line = line.replace(replaceLine2, "      ADD_MODULE_ALGORITHM(" + name + ", " + name + "Algo)\n" + replaceLine2)
		sys.stdout.write(line)
	print("algo factory edit: ", factorypath, name)

def makeUIFile(rootpath, name, filepath, extension, templatefilepath):
	uiPath = os.path.join(rootpath, filepath)
	uiFile = os.path.join(uiPath, name + extension)
	templatefile = os.path.join(uiPath, templatefilepath)
	makeFileFromTemplate(templatefile, uiFile, "@ModuleName@", name)
	print("UI file: ", uiFile)

def makeUIDesignerFile(path, name):
	makeUIFile(path, name, "Interface/Modules/Fields/", "Dialog.ui", "../Template/ModuleDesignerFile.ui")

def makeUIHeaderFile(path, name):
	makeUIFile(path, name, "Interface/Modules/Fields/", "Dialog.h", "../Template/ModuleDialog.h")

def makeUISourceFile(path, name):
	makeUIFile(path, name, "Interface/Modules/Fields/", "Dialog.cc", "../Template/ModuleDialog.cc")

def editUICMake(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Fields/")
	cmakeFile = os.path.join(uiPath, "CMakeLists.txt")
	replaceLineSrc = "SET(Interface_Modules_Fields_SOURCES"
	replaceLineHeader = "SET(Interface_Modules_Fields_HEADERS"
	replaceLineUI = "SET(Interface_Modules_Fields_FORMS"
	editCMake(cmakeFile, name + "Dialog", replaceLineSrc, replaceLineHeader, replaceLineUI)
	print("UI cmake edit: ", uiPath, name)

def addUIToFactory(path, name):
	uiPath = os.path.join(path, "Interface/Modules/Factory/ModuleDialogFactory.cc")
	replaceLine1 = "#include <boost/assign.hpp>"
	replaceLine2 = "  ;"
	for line in fileinput.input(uiPath, inplace=True):
		line = line.replace(replaceLine1, "#include <Interface/Modules/Fields/" + name + "Dialog.h>\n" + replaceLine1)
		line = line.replace(replaceLine2, "    ADD_MODULE_DIALOG(" + name + ", " + name + "Dialog)\n" + replaceLine2)
		sys.stdout.write(line)
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
	editAlgorithmTestCMake(path, name)
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
	print("Usage:", sys.argv[0], " moduleName [--ui]"); sys.exit(1)

restofargs = sys.argv[2:]
makeUI = "--ui" in restofargs

if not makeUI:
	print("UI flag not specified, not generating UI files")

srcRoot = os.path.abspath(os.path.join(os.getcwd(), '../../'))

makeModuleFiles(srcRoot, moduleName)
makeAlgorithmFiles(srcRoot, moduleName)
if makeUI:
	makeUIFiles(srcRoot, moduleName)
