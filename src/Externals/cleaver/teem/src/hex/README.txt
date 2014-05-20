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

-----------
enhex/dehex
-----------

enhex and dehex are a stand-alone hex encoder/decoder pair.
They convert between data in raw form (paying no regard to the
endianness), and convert it to hexadecimal form.  

To compile:

  cc -o enhex enhex.c
  cc -o dehex dehex.c

I wrote these programs for doing the hex encoding which is an optional
encoding in the nrrd library in Teem.  The (convoluted) reason is that
the only encodings that are *required* of non-Teem nrrd readers and
writers are raw and ascii.  All other encodings should be able to be
handled by stand-alone tools (such as gzip/gunzip for zlib
compression, and bzip/bunzip2 for bzip compress).  Yet, a google
search didn't reveal simple tools for raw--hex conversion, so I wrote
them.  Nothing fancy.

Gordon Kindlmann
