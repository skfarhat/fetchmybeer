file(GLOB SOURCE_FILES "*.hpp" "*.cpp" )

add_executable(main ${SOURCE_FILES})

target_link_libraries( main ${OpenCV_LIBS} )
target_compile_features(main PRIVATE cxx_range_for)