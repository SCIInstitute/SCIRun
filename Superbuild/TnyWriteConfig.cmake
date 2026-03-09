
set(_cfg_dir "C:/Users/Yong/Documents/SCIRun/build/Externals/Install/Tny_external/lib/cmake/Tny")
file(MAKE_DIRECTORY "${_cfg_dir}")
configure_file("C:/Users/Yong/Documents/SCIRun/Superbuild/TnyConfig.cmake.in" "${_cfg_dir}/TnyConfig.cmake" @ONLY)
