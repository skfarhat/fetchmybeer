file(GLOB SOURCE_FILES "*.hpp" "*.cpp" )

add_executable(test-color ${SOURCE_FILES})

target_link_libraries( test-color ${OpenCV_LIBS} )
target_compile_features(test-color PRIVATE cxx_range_for)