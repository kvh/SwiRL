# Copyright (C) 2006 Mihai Surdeanu <surdeanu@gmail.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT([swirl],
        [1.1.0],
        [Mihai Surdeanu surdeanu@gmail.com],
        [swirl])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_CONFIG_FILES([
   Makefile
   README
   m4/Makefile
   src/Makefile
   src/lib/Makefile
   src/charniak/Makefile
   src/ab/Makefile
   src/bin/Makefile
   data/Makefile
   model_charniak/Makefile 
   model_swirl/Makefile
   corpus/Makefile
])

AC_OUTPUT
