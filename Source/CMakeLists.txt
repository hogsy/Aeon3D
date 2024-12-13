add_library(Core STATIC
        World/Fog.c
        World/Frustum.c
        World/Gbspfile.c
        World/Light.c
        World/Plane.c
        World/Surface.c
        World/Trace.c
        World/User.c
        World/Vis.c
        World/WBitmap.c
        World/World.c

        Engine/Logo/A_CORONA.c
        Engine/Logo/A_STREAK.c
        Engine/Logo/CORONA.c
        Engine/Logo/electric.c
        Engine/Logo/logo.c
        Engine/Logo/LogoActor.c
        Engine/Logo/streak.c
        Engine/Logo/WebUrl.c
        Engine/BitmapList.c
        Engine/engine.c
        Engine/fontbmp.c
        Engine/System.c

        Actor/actor.c
        Actor/body.c
        Actor/bodyinst.c
        Actor/motion.c
        Actor/path.c
        Actor/pose.c
        Actor/puppet.c
        Actor/QKFrame.c
        Actor/strblock.c
        Actor/tkarray.c
        Actor/tkevents.c
        Actor/vkframe.c
        Actor/XFArray.c

        Math/Box.c
        Math/crc32.c
        Math/ExtBox.c
        Math/quatern.c
        Math/VEC3D.C
        Math/Xform3d.c

        Entities/Entities.c

        Support/ERRORLOG.C
        Support/geAssert.c
        Support/log.c
        Support/mempool.c
        Support/RAM.C

        Physics/matrix33.c
        Physics/PhysicsJoint.c
        Physics/PhysicsObject.c
        Physics/PhysicsSystem.c

        VFile/dirtree.c
        VFile/fsdos.c
        VFile/FSMEMORY.c
        VFile/fsvfs.c
        VFile/vfile.c

        Bitmap/Compression/palcreate.c
        Bitmap/Compression/palettize.c
        Bitmap/Compression/paloptimize.c
        Bitmap/Compression/yuv.c
        Bitmap/bitmap.c
        Bitmap/bitmap_blitdata.c
        Bitmap/bitmap_gamma.c
        Bitmap/pixelformat.c

        Font/font.c
        Font/wgClip.c

        Camera.c
        CSNetMgr.c
        Ge.c
        list.c
        NETPLAY.C
        sound.c
        Sound3d.c
        Tclip.c
        timer.c
        tsc.c
)

# Because of the mixed-case, CMake/CLion seem to be getting confused and assuming these are C++??
# We'll fix that by explicitly specifying them as C sources for now...
set_source_files_properties(
        VFile/FSMEMORY.c
        PROPERTIES LANGUAGE C
)

# ewwwwww need to restructure all this sometime...
target_include_directories(Core PUBLIC
        ./
        ./World
        ./Engine
        ./Engine/Drivers
        ./Actor
        ./Math
        ./Entities
        ./Support
        ./Physics
        ./VFile
        ./Bitmap
        ./Bitmap/Compression
)

target_compile_definitions(Core PRIVATE
        BUILDGENESIS
)
