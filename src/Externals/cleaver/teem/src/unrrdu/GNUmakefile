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
L := unrrdu
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
$(L).NEED = nrrd biff hest air
$(L).PUBLIC_HEADERS = unrrdu.h
$(L).PRIVATE_HEADERS = privateUnrrdu.h
$(L).OBJS = flotsam.o
#
# The source for unu command xxx must define a unuCmd "unu_xxxCmd".
# This applies to every .o file below.
#
$(L).OBJS  += flip.o slice.o convert.o crop.o pad.o permute.o histo.o \
	resample.o cmedian.o reshape.o shuffle.o minmax.o quantize.o \
	unquantize.o block.o unblock.o project.o swap.o join.o dhisto.o \
	jhisto.o dice.o heq.o histax.o gamma.o make.o 1op.o 2op.o 3op.o \
	lut.o subst.o rmap.o imap.o lut2.o save.o head.o data.o splice.o \
	inset.o axinsert.o axdelete.o axinfo.o ccfind.o ccadj.o ccmerge.o \
	ccsettle.o about.o axsplit.o axmerge.o mlut.o mrmap.o tile.o untile.o \
	unorient.o env.o dist.o
####
####
####

# boilerplate: declare rules for this library
include $(TEEM_SRC)/make/template.mk
endif
ifeq (,$(INCLUDED))
  include $(TEEM_SRC)/bin/GNUmakefile
endif
