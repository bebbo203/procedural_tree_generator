add_subdirectory(yscenetrace)
add_subdirectory(test_app)

if(YOCTO_OPENGL)
add_subdirectory(ysceneitraces)
endif(YOCTO_OPENGL)
