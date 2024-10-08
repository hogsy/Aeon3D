add_executable(GBSPCmd GBSPCmd.cpp)
target_link_libraries(GBSPCmd Core)
add_dependencies(GBSPCmd GBSPLib)
