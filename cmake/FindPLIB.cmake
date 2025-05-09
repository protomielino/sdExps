# - Locate PLib
# This module defines
# PLIB_UL_LIBRARY, PLIB_SG_LIBRARY, PLIB_SL_LIBRARY
# PLIB_SSG_LIBRARY, PLIB_SSGAUX_LIBRARY
#PLIB_LIBRARY
# PLIB_FOUND, if false, do not try to link to PLib
# PLIB_INCLUDE_DIR, where to find the headers
#
# $PLIB_DIR is an environment variable that would
# correspond to the ./configure --prefix=$PLIB_DIR
# used in building PLib.
#
# Created by Mart Kelder.

IF(NOT APPLE OR OPTION_USE_MACPORTS)
 FIND_PATH(PLIB_PLIBINCLUDE_DIR plib/ssg.h
  HINTS ENV PLIB_DIR
  PATH_SUFFIXES
	Headers include/plib include develop/headers/plib develop/headers/x86/plib
  PATHS
  #Additional MacOS Paths
   	 ~/Library/Frameworks/plib.framework
  	 /Library/Frameworks/plib.framework
 	 /System/Library/Frameworks/plib.framework # Tiger

	/usr /usr/local
   /system
  DOC "Location of plib")
ENDIF(NOT APPLE OR OPTION_USE_MACPORTS)

IF(APPLE)
   IF(NOT OPTION_USE_MACPORTS)
    FIND_PATH(PLIB_PLIBINCLUDE_DIR ssg.h
     HINTS ENV PLIB_DIR
     PATH_SUFFIXES
	   Headers include/plib include
     PATHS
      #Additional MacOS Paths
   	    ~/Library/Frameworks/plib.framework
  	    /Library/Frameworks/plib.framework
 	    /System/Library/Frameworks/plib.framework # Tiger

	   /usr /usr/local
     DOC "Location of plib")
   ENDIF(NOT OPTION_USE_MACPORTS)
ENDIF(APPLE)

SET(PLIB_INCLUDE_DIR ${PLIB_PLIBINCLUDE_DIR} CACHE STRING "Include dir for plib")

SET(PLIB_ALREADY_FOUND FALSE)
IF(OPTION_USE_MACPORTS)
   IF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
	   SET(PLIB_ALREADY_FOUND TRUE)
   ENDIF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
ELSE(OPTION_USE_MACPORTS)
   IF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SL_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
	   SET(PLIB_ALREADY_FOUND TRUE)
   ENDIF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SL_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
ENDIF(OPTION_USE_MACPORTS)

IF (NOT APPLE OR OPTION_USE_MACPORTS)
  FIND_LIBRARY(PLIB_UL_LIBRARY
	NAMES plibul ul
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
  FIND_LIBRARY(PLIB_JS_LIBRARY
	NAMES plibjs js
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
  FIND_LIBRARY(PLIB_SG_LIBRARY
	NAMES plibsg sg
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
IF(NOT OPTION_USE_MACPORTS)
  FIND_LIBRARY(PLIB_SL_LIBRARY
	NAMES plibsl sl
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
ENDIF(NOT OPTION_USE_MACPORTS)

  FIND_LIBRARY(PLIB_SSG_LIBRARY
	NAMES plibssg ssg
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
  FIND_LIBRARY(PLIB_SSGAUX_LIBRARY
	NAMES plibssgaux ssgaux ssgAux
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS /usr /usr/local)
ENDIF(NOT APPLE OR OPTION_USE_MACPORTS)

#special case plib is only one library in MacOS
IF (APPLE AND NOT OPTION_USE_MACPORTS)
  FIND_LIBRARY(PLIB_APPLE_LIBRARY
	NAMES plib
	HINTS ENV PLIB_DIR
	PATH_SUFFIXES lib64 lib libs64 libs
	PATHS /usr /usr/local)

 SET(PLIB_APPLE_LIBRARY ${PLIB_APPLE_LIBRARY} "-framework Cocoa")
ENDIF(APPLE AND NOT OPTION_USE_MACPORTS)


IF(NOT APPLE)
 IF(PLIB_INCLUDE_DIR AND
   PLIB_UL_LIBRARY AND
   PLIB_JS_LIBRARY AND
   PLIB_SG_LIBRARY AND
   PLIB_SL_LIBRARY AND
   PLIB_SSG_LIBRARY AND
   PLIB_SSGAUX_LIBRARY)
	SET(PLIB_FOUND TRUE)
 ENDIF(PLIB_INCLUDE_DIR AND
   PLIB_UL_LIBRARY AND
   PLIB_JS_LIBRARY AND
   PLIB_SG_LIBRARY AND
   PLIB_SL_LIBRARY AND
   PLIB_SSG_LIBRARY AND
   PLIB_SSGAUX_LIBRARY)
ELSEIF(OPTION_USE_MACPORTS)
   IF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_JS_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
	   SET(PLIB_FOUND TRUE)
    ENDIF(PLIB_INCLUDE_DIR AND
      PLIB_UL_LIBRARY AND
      PLIB_JS_LIBRARY AND
      PLIB_SG_LIBRARY AND
      PLIB_SSG_LIBRARY AND
      PLIB_SSGAUX_LIBRARY)
ENDIF()

IF(APPLE AND NOT OPTION_USE_MACPORTS)
 IF(PLIB_INCLUDE_DIR AND PLIB_APPLE_LIBRARY)
	SET(PLIB_FOUND TRUE)
 ENDIF(PLIB_INCLUDE_DIR AND PLIB_APPLE_LIBRARY)
ENDIF(APPLE AND NOT OPTION_USE_MACPORTS)

IF(PLIB_FOUND)
	IF(NOT PLIB_FIND_QUIETLY AND NOT PLIB_ALREADY_FOUND)
		MESSAGE(STATUS "Looking for PLib - found (${PLIB_SG_LIBRARY})")
	ENDIF(NOT PLIB_FIND_QUIETLY AND NOT PLIB_ALREADY_FOUND)
IF(OPTION_USE_MACPORTS)
	SET(PLIB_LIBRARY ${PLIB_UL_LIBRARY} ${PLIB_JS_LIBRARY} ${PLIB_SG_LIBRARY}
${PLIB_SSG_LIBRARY} ${PLIB_SSGAUX_LIBRARY})
ELSE(OPTION_USE_MACPORTS)
	SET(PLIB_LIBRARY ${PLIB_UL_LIBRARY} ${PLIB_JS_LIBRARY} ${PLIB_SG_LIBRARY} ${PLIB_SL_LIBRARY}
${PLIB_SSG_LIBRARY} ${PLIB_SSGAUX_LIBRARY})
ENDIF(OPTION_USE_MACPORTS)
ELSE(PLIB_FOUND)
	IF(PLIB_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find plib")
	ENDIF(PLIB_FIND_REQUIRED)
ENDIF(PLIB_FOUND)

IF(PLIB_FOUND)
	IF(EXISTS ${PLIB_INCLUDE_DIR}/plib/ul.h)
		FILE(READ ${PLIB_INCLUDE_DIR}/plib/ul.h PLIB_TMP_UL_H)
	ELSE(EXISTS ${PLIB_INCLUDE_DIR}/plib/ul.h)
		FILE(READ ${PLIB_INCLUDE_DIR}/ul.h PLIB_TMP_UL_H)
	ENDIF(EXISTS ${PLIB_INCLUDE_DIR}/plib/ul.h)
	STRING(REGEX REPLACE "^.*define[\t ]+PLIB_MAJOR_VERSION[\t ]+([0123456789]+)[^0123456789].*$" "\\1" PLIB_VERSION_MAJOR ${PLIB_TMP_UL_H})
	STRING(REGEX REPLACE "^.*define[\t ]+PLIB_MINOR_VERSION[\t ]+([0123456789]+)[^0123456789].*$" "\\1" PLIB_VERSION_MINOR ${PLIB_TMP_UL_H})
	STRING(REGEX REPLACE "^.*define[\t ]+PLIB_TINY_VERSION[\t ]+([0123456789]+)[^0123456789].*$"  "\\1" PLIB_VERSION_TINY ${PLIB_TMP_UL_H})
	SET(PLIB_VERSION "${PLIB_VERSION_MAJOR}${PLIB_VERSION_MINOR}${PLIB_VERSION_TINY}")
ENDIF(PLIB_FOUND)

MARK_AS_ADVANCED(PLIB_PLIBINCLUDE_DIR)
