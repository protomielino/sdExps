INCLUDE(../../../cmake/macros.cmake)

PROJECT(trackgen)

SET(_TARGET_NAME ${SD_TOOLS_EXECPREFIX}trackgen)

SET(_SOURCES ac3d.cpp easymesh.cpp elevation.cpp main.cpp
             objects.cpp raceline.cpp relief.cpp track.cpp util.cpp)
SET(_HEADERS ac3d.h easymesh.h elevation.h objects.h relief.h trackgen.h util.h)
SET(_OTHER_SOURCES trackgen.xml)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(robottools tgf tgfclient math portability)

ADD_SDL2_INCLUDEDIR()

ADD_PLIB_INCLUDEDIR()

#disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmtd.lib")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt.lib")
ENDIF(MSVC)

# Note: Headers and other sources needed for having them available in IDEs.
SD_ADD_EXECUTABLE(${_TARGET_NAME} ${_SOURCES} ${_HEADERS} ${_OTHER_SOURCES})

target_compile_features(${_TARGET_NAME} PRIVATE cxx_std_11)
set_target_properties(${_TARGET_NAME} PROPERTIES
    CXX_STANDARD 11 CXX_EXTENSIONS OFF)

ADD_SDLIB_LIBRARY(${_TARGET_NAME} portability robottools tgf tgfclient)

IF(OPENGL_FOUND)
    TARGET_LINK_LIBRARIES(${_TARGET_NAME} ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PNG_LIBRARY(${_TARGET_NAME})

ADD_SDL2_LIBRARY(${_TARGET_NAME})

ADD_PLIB_LIBRARY(${_TARGET_NAME} ssg sg ul)
ADD_DL_LIBRARY(${_TARGET_NAME})

# Needed when using the "MinGW Makefiles" CMake generator (but not with the "MSYS Makefiles" one !?)
IF(MINGW)
	TARGET_LINK_LIBRARIES(${_TARGET_NAME} ${MINGW32_LIBRARY})
ENDIF(MINGW)

IF(UNIX)
    SD_INSTALL_FILES(MAN man6 PREFIX ${CMAKE_SOURCE_DIR}/doc/man FILES ${_TARGET_NAME}.6)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS ${_TARGET_NAME})

IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.13")
   # To avoid 'undefined reference to glActiveTextureARB [--no-allow-shlib-undefined]'
   IF(UNIX AND NOT APPLE)
	   IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		    target_link_options(${_TARGET_NAME} PRIVATE -Wl,--allow-shlib-undefined)
	   ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   ENDIF(UNIX AND NOT APPLE)
ENDIF(NOT ${CMAKE_VERSION} VERSION_LESS "3.13")
