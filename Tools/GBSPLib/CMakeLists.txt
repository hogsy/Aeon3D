add_library(GBSPLib SHARED
        Brush2.cpp
        BSP.CPP
        Bsp2.cpp
        Fill.Cpp
        GBSPFILE.CPP
        Gbsplib.cpp
        GBSPPREP.CPP
        Leaf.cpp
        LIGHT.CPP
        MAP.CPP
        MATHLIB.CPP
        POLY.CPP
        PORTALS.CPP
        PortFile.cpp
        RAD.CPP
        TEXTURE.CPP
        TJunct.cpp
        Utils.cpp
        VIS.CPP
        VISFLOOD.CPP
)

# Because of the mixed-case, CMake/CLion seem to be getting confused and assuming these are C++??
# We'll fix that by explicitly specifying them as C sources for now...
set_source_files_properties(
        Fill.Cpp
        PROPERTIES LANGUAGE CXX
)

target_link_libraries(GBSPLib PRIVATE Core)
