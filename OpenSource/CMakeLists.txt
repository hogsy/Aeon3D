cmake_minimum_required(VERSION 3.5.1)

project(HG3D
        DESCRIPTION "3D game engine forked from Genesis3D."
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
foreach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR})
endforeach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})

add_subdirectory(Source)
add_subdirectory(GTest)
#add_subdirectory(GDemo)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

# 3D Drivers
add_subdirectory(Source/Engine/Drivers/GLDrv)
if (WIN32)
    add_subdirectory(Source/Engine/Drivers/D3DDrv)
    #add_subdirectory(Source/Engine/Drivers/GlideDrv)
endif ()
