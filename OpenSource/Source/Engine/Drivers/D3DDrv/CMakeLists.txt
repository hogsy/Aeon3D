add_library(D3DDrv SHARED
        D3dcache.cpp
        D3ddrv.cpp
        D3D_ERR.CPP
        D3d_fx.cpp
        D3d_main.cpp
        DDMemMgr.c
        GSPAN.CPP
        Pcache.cpp
        Render.cpp
        Scene.cpp
        THandle.cpp
        tpage.cpp
)

target_include_directories(D3DDrv PRIVATE
        ../
        ../../../
        ../../../Bitmap
        ../../../Math
        ../../../Support
)
