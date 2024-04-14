cmake_minimum_required(VERSION 3.5.1)

project(HG3D
        DESCRIPTION "3D game engine forked from Genesis3D"
)

set(CMAKE_C_STANDARD 11)

add_subdirectory(Source)
add_subdirectory(GTest)

# 3D Drivers
#[[
if (WIN32)
    add_subdirectory(Source/Engine/Drivers/D3DDrv)
    add_subdirectory(Source/Engine/Drivers/GlideDrv)
endif ()
]]
