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
L := moss
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
$(L).NEED = ell nrrd biff hest air
$(L).PUBLIC_HEADERS = moss.h
$(L).PRIVATE_HEADERS = privateMoss.h
$(L).OBJS = defaultsMoss.o methodsMoss.o sampler.o xform.o hestMoss.o
$(L).TESTS = test/invert
####
####
####

# boilerplate: declare rules for this library
include $(TEEM_SRC)/make/template.mk
endif
ifeq (,$(INCLUDED))
  include $(TEEM_SRC)/bin/GNUmakefile
endif
