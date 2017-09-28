cmake_minimum_required(VERSION 3.6)

if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()

project(urho3dSamples)

set(CMAKE_CXX_STANDARD 11)

# Set URHO3D path
# TODO: Set path to your urho3D build path
set(URHO3D_HOME "${CMAKE_SOURCE_DIR}/Urho3D-1.7/build")

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)

# Samples
add_subdirectory(samples/00-emptyProject)
add_subdirectory(samples/01-helloWorld)
add_subdirectory(samples/02-helloGUI)
add_subdirectory(samples/03-sprites)
add_subdirectory(samples/04-staticScene)
add_subdirectory(samples/05-animatingScene)
add_subdirectory(samples/06-skeletalAnimation)
add_subdirectory(samples/07-renderToTexture)
add_subdirectory(samples/08-physics)
add_subdirectory(samples/09-ragdolls)
add_subdirectory(samples/10-SoundEffects)
add_subdirectory(samples/11-navigation)
add_subdirectory(samples/12-vehicle)