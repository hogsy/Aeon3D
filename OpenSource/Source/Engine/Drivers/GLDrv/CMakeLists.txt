# OpenGL Driver for HG3D

add_library(GLDrv SHARED
        GlideDrv.c
        GMain.c
        GThandle.c
        Render.c

        ../Bmp.c

        glew-2.1.0/src/glew.c
)

target_include_directories(GLDrv PRIVATE
        ../
        ../../../
        ../../../Bitmap
        ../../../Math
        ../../../Support

        glew-2.1.0/include
)

target_compile_definitions(GLDrv PRIVATE
        GLEW_STATIC
        __MSC__
)
target_link_libraries(GLDrv OpenGL32)
