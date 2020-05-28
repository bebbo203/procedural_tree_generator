add_executable(tree_generator tree_generator.cpp)

set_target_properties(tree_generator PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(tree_generator PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(tree_generator yocto yocto_gui yocto_pathtrace yocto_extension )
