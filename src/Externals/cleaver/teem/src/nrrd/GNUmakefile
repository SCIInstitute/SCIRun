#
# Teem: Tools to process and visualize scientific data and images
# Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# (LGPL) as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The terms of redistributing and/or modifying this software also
# include exceptions to the LGPL that facilitate static linking.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

#### Library name
####
####
L := nrrd
####
####
####

# boilerplate: default targets and include tricks
TEEM_ROOT ?= ../..
TEEM_SRC ?= ..
ifeq (,$(DEF_TARGETS))
DEF_TARGETS = true
dev     : $(L)/dev
install : $(L)/install
clean   : $(L)/clean
clobber : $(L)/clobber
include ../GNUmakefile
endif
ifeq (,$($(L).SEEN))
$(L).SEEN := true

#### Describe library here
####
####
$(L).NEED = biff hest air   # hest needed for callback declaration
$(L).PUBLIC_HEADERS = nrrd.h nrrdDefines.h nrrdMacros.h nrrdEnums.h
$(L).PRIVATE_HEADERS = privateNrrd.h
$(L).OBJS = \
	accessors.o  arith.o     arraysNrrd.o   apply1D.o apply2D.o \
	axis.o       comment.o   convertNrrd.o  defaultsNrrd.o   \
	endianNrrd.o enumsNrrd.o filt.o         gzio.o           \
	hestNrrd.o   histogram.o iter.o         kernel.o         \
	map.o        measure.o   methodsNrrd.o  parseNrrd.o      \
	read.o       write.o        reorder.o   resampleNrrd.o \
	simple.o     subset.o     superset.o  tmfKernel.o      \
	winKernel.o  ccmethods.o  cc.o        range.o          \
        encoding.o   encodingRaw.o  encodingAscii.o  encodingHex.o \
	encodingGzip.o   encodingBzip2.o  \
	format.o     formatNRRD.o     formatPNM.o      formatPNG.o \
	formatVTK.o      formatText.o     formatEPS.o      \
	keyvalue.o  resampleContext.o
$(L).TESTS = test/tread test/trand test/ax test/io test/strio \
	test/minmax test/tkernel test/typestest test/tline test/genvol \
	test/quadvol test/convo test/kv test/reuse test/histrad test/otsu

####
####
####

# boilerplate: declare rules for this library, do other libraries + bins
include $(TEEM_SRC)/make/template.mk
endif
ifeq (,$(INCLUDED))
  include $(TEEM_SRC)/bin/GNUmakefile
endif
