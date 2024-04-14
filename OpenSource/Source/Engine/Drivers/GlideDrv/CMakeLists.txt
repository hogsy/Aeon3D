
add_library(GlideDrv SHARED
        GCache.c
        GlideDrv.c
        GMain.c
        GMemMgr.c
        GSpan.cpp
        GThandle.c
        Render.c

        ../Bmp.c
)

target_include_directories(GlideDrv PRIVATE
        ../
        ../../../
        ../../../Bitmap
        ../../../Math
        ../../../Support

        Glide2x/Src/Include
)

target_compile_definitions(GlideDrv PRIVATE __MSC__)

target_link_directories(GlideDrv PRIVATE Glide2x/Src/Lib/Win32)
target_link_libraries(GlideDrv glide2x)
