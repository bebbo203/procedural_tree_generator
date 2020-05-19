add_executable(test_app test_app.cpp)

set_target_properties(test_app PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(test_app PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(test_app yocto yocto_gui yocto_pathtrace yocto_extension)
