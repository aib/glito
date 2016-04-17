dnl -*- sh -*-
# glito/configure.ac  v1.0  2002.04.03
# Copyright (C) 1996,2002 Emmanuel Debanne
#  
#    Configure file for Glito.
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


#AC_COPYRIGHT(Copyright (C) 2002 Emmanuel Debanne. Distributed under the terms of the GNU General Public License.)

AC_INIT( glito, 1.0b1, glito@debanne.net )
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(src/Main.cpp)

AM_INIT_AUTOMAKE( glito, 1.0b1 )

ALL_LINGUAS="fr eo"

AC_LANG(C++)

AC_PROG_CXX

AC_CHECK_PROG( FLTKCONFIG, fltk-config, 1, 0 )
if test $FLTKCONFIG -eq 0; then
    AC_MSG_ERROR(Can not find fltk-config. Fltk must be installed (http://www.fltk.org/)!)
fi

AC_CHECK_HEADER( Fl/Fl.H,
		 LIBS="$LIBS `fltk-config --use-images --ldflags`"
		 CPPFLAGS="$CPPFLAGS `fltk-config --cxxflags`",
		 AC_MSG_ERROR(Can not find Fltk includes. Fltk must be installed (http://www.fltk.org/)!) )

# getopt support:
AC_CHECK_HEADERS(unistd.h)

AC_CHECK_LIB( png, png_init_io, ,
	      AC_MSG_WARN(You will not have any PNG support!) )

AC_CHECK_LIB( mng, mng_initialize, ,
	      AC_MSG_WARN(You will not have any MNG support!) )

AC_CHECK_LIB( z, compress2, , AC_MSG_ERROR(Cannot find zlib. (Try installing the package libz-dev.)) )

AM_GNU_GETTEXT

AC_CONFIG_FILES( Makefile intl/Makefile po/Makefile.in \
    po/Makefile src/Makefile doc/Makefile )

AC_OUTPUT