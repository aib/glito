# glito/src/Makefile.am  v1.0  2002.04.03
# Copyright (C) 1996,2002 Emmanuel Debanne
#  
#    Makefile for Glito.
#    Glito is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#    Glito is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#    You should have received a copy of the GNU General Public License
# along with Glito (named COPYING); if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#

bin_PROGRAMS = glito

glito_SOURCES = \
	Formula.cpp IndentedString.cpp ImageGray.cpp Image.cpp Function.cpp Skeleton.cpp Engine.cpp Glito.cpp \
	Formula.hpp IndentedString.hpp ImageGray.hpp Image.hpp Function.hpp Skeleton.hpp Engine.hpp Glito.hpp \
	Main.cpp

glito_LDADD = @INTLLIBS@

datadir = @datadir@
docdir = $(datadir)/doc/glito/
localedir = $(datadir)/locale

DEFS = -DDOCDIR=\"$(docdir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

INCLUDES = -I.. -I../intl

AM_CXXFLAGS = -fpermissive
