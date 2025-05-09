INCLUDE(../../../cmake/macros.cmake)

PROJECT(accc)

SET(_TARGET_NAME ${SD_TOOLS_EXECPREFIX}accc)

SET(ACCC_SOURCES ac3dgroup.cpp ac3dload.cpp mainaccc.cpp)
SET(ACCC_HEADERS accc.h)

ADD_SDLIB_INCLUDEDIR(portability)

# Note: Headers needed for having them available in IDEs.
SD_ADD_EXECUTABLE(${_TARGET_NAME} ${ACCC_SOURCES} ${ACCC_HEADERS})
target_compile_features(${_TARGET_NAME} PRIVATE cxx_std_11)
set_target_properties(${_TARGET_NAME} PROPERTIES CXX_STANDARD 11 CXX_EXTENSIONS OFF)

ADD_SDLIB_LIBRARY(${_TARGET_NAME} portability)

IF(UNIX)
    SD_INSTALL_FILES(MAN man6 PREFIX ${CMAKE_SOURCE_DIR}/doc/man FILES ${_TARGET_NAME}.6)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS ${_TARGET_NAME})
