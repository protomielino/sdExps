INCLUDE(../../../../cmake/macros.cmake)

SET(_SOURCES textonly.cpp)
SET(_HEADERS textonly.h)

IF(OPTION_CLIENT_SERVER)
   SET(NETWORKING_INC csnetworking)
   SET(NETWORKING_LIB csnetworking)
ELSE(OPTION_CLIENT_SERVER)
   SET(NETWORKING_INC networking)
   SET(NETWORKING_LIB networking)
ENDIF(OPTION_CLIENT_SERVER)

#INCLUDE_DIRECTORIES(.)
ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math portability tgf tgfclient tgfdata ${NETWORKING_INC})
ADD_PLIB_INCLUDEDIR()

ADD_SDL2_INCLUDEDIR()

ADD_ENET_INCLUDEDIR()

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DTEXTONLY_DLL)
ENDIF(WIN32)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(MSVC)

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# Note: Headers needed for having them available in IDEs.
SD_ADD_LIBRARY(textonly MODULE ${_SOURCES} ${_HEADERS})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose)
#SET_TARGET_PROPERTIES(textonly PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_SDLIB_LIBRARY(textonly portability tgf tgfclient tgfdata ${NETWORKING_LIB})

SD_INSTALL_FILES(LIB modules/userinterface TARGETS textonly)
