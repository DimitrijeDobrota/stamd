# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(stamdTests LANGUAGES CXX)

# ---- Tests ----

add_executable(stamd_test source/stamd_test.cpp)
target_link_libraries(stamd_test PRIVATE stamd_lib)
target_compile_features(stamd_test PRIVATE cxx_std_20)

add_test(NAME stamd_test COMMAND stamd_test)

# ---- End-of-file commands ----

add_folders(Test)
