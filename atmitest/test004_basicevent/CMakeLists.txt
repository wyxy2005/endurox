## 
## Basic Event Testing
##
## @file CMakeLists.txt
## 
## -----------------------------------------------------------------------------
## Enduro/X Middleware Platform for Distributed Transaction Processing
## Copyright (C) 2015, ATR Baltic, SIA. All Rights Reserved.
## This software is released under one of the following licenses:
## GPL or ATR Baltic's license for commercial use.
## -----------------------------------------------------------------------------
## GPL license:
## 
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
## PARTICULAR PURPOSE. See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the Free Software Foundation, Inc., 59 Temple
## Place, Suite 330, Boston, MA 02111-1307 USA
##
## -----------------------------------------------------------------------------
## A commercial use license is available from ATR Baltic, SIA
## contact@atrbaltic.com
## -----------------------------------------------------------------------------
##
cmake_minimum_required(VERSION 2.8)

# Make sure the compiler can find include files from UBF library
include_directories (${ENDUROX_SOURCE_DIR}/libubf
					 ${ENDUROX_SOURCE_DIR}/include
					 ${ENDUROX_SOURCE_DIR}/libnstd
					 ${ENDUROX_SOURCE_DIR}/ubftest)
# Add debug options
# By default if RELEASE_BUILD is not defined, then we run in debug!
IF ($ENV{RELEASE_BUILD})
	# do nothing
ELSE ($ENV{RELEASE_BUILD})
	ADD_DEFINITIONS("-D NDRX_DEBUG")
ENDIF ($ENV{RELEASE_BUILD})

# Make sure the linker can find the UBF library once it is built.
link_directories (${ENDUROX_BINARY_DIR}/libubf) 

############################# Test - executables ###############################
add_executable (atmisv4_1ST atmisv4_1ST.c ../../libatmisrv/rawmain_integra.c)
add_executable (atmisv4_2ND atmisv4_2ND.c ../../libatmisrv/rawmain_integra.c)
add_executable (atmiclt4 atmiclt4.c)
################################################################################
############################# Test - executables ###############################
# Link the executable to the ATMI library & others...
target_link_libraries (atmisv4_1ST m ubf nstd atmi atmisrvinteg ${RT_LIB} pthread)
target_link_libraries (atmisv4_2ND m ubf nstd atmi atmisrvinteg ${RT_LIB} pthread)
target_link_libraries (atmiclt4 m ubf nstd atmi atmiclt ${RT_LIB} pthread)

set_target_properties(atmisv4_1ST PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")
set_target_properties(atmisv4_2ND PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")
set_target_properties(atmiclt4 PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")

################################################################################
