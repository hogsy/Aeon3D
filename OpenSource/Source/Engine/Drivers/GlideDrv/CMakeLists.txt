
add_library(GlideDrv SHARED
        GCache.c
        GlideDrv.c
        GMain.c
        GMemMgr.c
        GSpan.cpp
        GThandle.c
        Render.c
)

target_include_directories(GlideDrv PRIVATE
        ../
        ../../../
        ../../../Bitmap
        ../../../Math
        ../../../Support

        3rdParty/Glide2x/Src/Include
)

target_link_directories(GlideDrv PRIVATE 3rdParty/Glide2x/Src/Lib/Win32)
target_link_libraries(GlideDrv -L -lglide2x)
