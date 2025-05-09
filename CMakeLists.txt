#CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)
CMAKE_MINIMUM_REQUIRED(VERSION 3.12 FATAL_ERROR)

# Speed Dreams project
PROJECT(speed-dreams-2 VERSION 2.4.0
   HOMEPAGE_URL "https://www.speed-dreams.net/"
)

MESSAGE(STATUS CMAKE_VERSION = "${CMAKE_VERSION}")
SET(BUILD_SHARED_LIBS ON)

IF(POLICY CMP0003)
   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(POLICY CMP0003)

IF(POLICY CMP0048)
   CMAKE_POLICY(SET CMP0048 NEW)
ENDIF(POLICY CMP0048)

IF(POLICY CMP0056)
   CMAKE_POLICY(SET CMP0056 NEW)
ENDIF(POLICY CMP0056)

IF(POLICY CMP0066)
  CMAKE_POLICY(SET CMP0066 NEW)
ENDIF(POLICY CMP0066)

IF(POLICY CMP0072)
  CMAKE_POLICY(SET CMP0072 NEW)
ENDIF(POLICY CMP0072)

IF(APPLE)
   SET(CMAKE_CXX_STANDARD 11)
   SET(CMAKE_CXX_STANDARD_REQUIRED True)
ENDIF(APPLE)

option(SD_ASSUME_DATADIR "Assume default directory for speed-dreams-data")
set(data_version 2.4.0)
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

if(SD_ASSUME_DATADIR)
   set(SD_DATADIR ${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
   set(SD_DATADIR_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
   set(SD_DATADIR_ABS ${SD_DATADIR_INSTALL_PREFIX})
   message(STATUS "Data directory: ${SD_DATADIR_ABS} (assumed)")
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/speed-dreams-data/CMakeLists.txt)
   add_subdirectory(speed-dreams-data)
   set(SD_DATADIR ${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
   set(SD_DATADIR_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
   set(SD_DATADIR_ABS ${CMAKE_CURRENT_SOURCE_DIR}/speed-dreams-data/data)
   message(STATUS "Data directory: ${SD_DATADIR_ABS} (in-tree)")
else()
   find_package(speed-dreams-data ${data_version} QUIET)

   if(NOT SD_DATADIR)
      message(FATAL_ERROR
         "Data directory not found or incompatible with version "
         "${data_version}. Please perform one of the following actions:\n"
         "- Update the data git submodule to do an in-tree build with the "
         " command below, and then configure the project again:\n"
         "git submodule update --init --recursive\n"
         "- If speed-dreams-data has been installed to a non-standard "
         "location, configure the project with "
         "-DCMAKE_PREFIX_PATH=<data-dir>.\n"
         "- Configure the project with -DSD_ASSUME_DATADIR=1 to assume "
         "speed-dreams-data was installed into a standard location. "
         "Speed Dreams should then still be able to build, but it might "
         "not run successfully. This option is usually only recommended "
         "for CI/CD and packagers."
      )
   endif()

   message(STATUS "Data directory: ${SD_DATADIR_ABS}")
endif()

find_package(SOLID 2.1.2)

if(NOT SOLID_FOUND)
   message(STATUS "SOLID not found. Using in-tree submodule.")
   add_subdirectory(freesolid)
endif()

if(CMAKE_CXX_COMPILER_ID)
   set(SD_BUILD_INFO_COMPILER_NAME ${CMAKE_CXX_COMPILER_ID})
else()
   set(SD_BUILD_INFO_COMPILER_NAME "Unknown")
endif()

INCLUDE(cmake/macros.cmake)
INCLUDE(cmake/checks.cmake)

set(CMAKE_INSTALL_RPATH
   ${CMAKE_INSTALL_RPATH}
   ${CMAKE_INSTALL_PREFIX}/${SD_LIBDIR}/lib
)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

CHECK_HEADERS_H()
CHECK_FUNCTIONS()
CHECK_LIBRARIES()

execute_process(
   COMMAND git describe --tags --dirty
   RESULT_VARIABLE result
   OUTPUT_VARIABLE VERSION
   OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT result EQUAL 0)
   set(VERSION "undefined-version")
endif()

set(VERSION_LONG ${VERSION} CACHE STRING "The version string displayed")

# CMAKE_PROJECT_VERSION_TWEAK is used by NSIS packagers
# its what comes after the '-' sign (if anything)
# ie: in tag '2.2.2-rc1' CMAKE_PROJECT_VERSION_TWEAK should be '-rc1'
STRING(FIND "${VERSION}" "-" _METADATA_INDEX)
IF(NOT ${_METADATA_INDEX} EQUAL -1)
   STRING(SUBSTRING ${VERSION} ${_METADATA_INDEX} -1 CMAKE_PROJECT_VERSION_TWEAK)
ELSE()
   SET(CMAKE_PROJECT_VERSION_TWEAK)
ENDIF()
#MESSAGE(STATUS "CMAKE_PROJECT_VERSION_TWEAK ${CMAKE_PROJECT_VERSION_TWEAK}")

# Generate config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Generate DOxygen configuration file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/doxygenconfig.in.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen_config @ONLY)

# Setup empty GLOBAL lists for robots and modules
SET_PROPERTY(GLOBAL PROPERTY SD_MODULE_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_CAR_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_TRACK_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_ROBOT_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_OSG_PLUGIN_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_EXE_LIST "")

# Sub-dirs to build ...
ADD_SUBDIRECTORY(src)

IF(MSVC)
   INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/faq/faq.html DESTINATION ./doc)
   INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/userman/how_to_drive.html DESTINATION ./doc)
   INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/userman/images/ DESTINATION ./doc/images/)
ENDIF(MSVC)

if(WIN32)
   function(copy_dll target)
      add_custom_target(${target}_copydll)
      add_dependencies(${target}_copydll ${target})
      add_dependencies(${PROJECT_NAME} ${target}_copydll)
      add_custom_command(TARGET ${target}_copydll
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${target}> ${CMAKE_BINARY_DIR}/bin/
         VERBATIM
      )
   endfunction()

   copy_dll(solid)
   copy_dll(broad)
   copy_dll(moto)
endif()

# Must be the last ADD_SUBDIRECTORY for FIXUP_BUNDLE to work
ADD_SUBDIRECTORY(packaging)

IF(OPTION_CHECK_CONTENTS)
   INCLUDE(cmake/prerelease.cmake)
ENDIF(OPTION_CHECK_CONTENTS)

INCLUDE(cpack/common.cmake)
INCLUDE(CPack)
