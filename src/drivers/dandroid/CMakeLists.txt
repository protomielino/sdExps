INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "dandroid")

SET(ROBOT_INTERFACE LEGACY WELCOME)

SET(ROBOT_SOURCES
    src/ClothoidPath.cpp
    src/ClothoidPath.h
    src/cubic.cpp
    src/cubic.h
    src/dandroid.cpp
    src/danpath.cpp
    src/danpath.h
    src/driver.cpp
    src/driver.h
    src/globaldefinitions.h
    src/LinePath.cpp
    src/LinePath.h
    src/MyTrack.cpp
    src/MyTrack.h
    src/opponent.cpp
    src/opponent.h
    src/pidcontroller.cpp
    src/pidcontroller.h
    src/pit.cpp
    src/pit.h
    src/Seg.h
    src/spline.cpp
    src/spline.h
    src/torcs_or_sd.h
    src/Utils.cpp
    src/Utils.h
    src/Vec2d.h
    src/Vec3d.h
)

# The ubiquitous robot module and its clones.
ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION 2.0.0 SOVERSION 1.0.0
             INTERFACE ${ROBOT_INTERFACE}
             SOURCES ${ROBOT_SOURCES})

target_compile_features(${ROBOT_NAME} PRIVATE cxx_std_11)
set_target_properties(${ROBOT_NAME} PROPERTIES
    CXX_STANDARD 11 CXX_EXTENSIONS OFF)

# For data associated to the ubiquitous robot module and its clones,
# see data/drivers/dandroid*/CMakeLists.txt
