file(GLOB SRCS "*.cpp" "*.hpp")

add_executable(color-thresholder "main.cpp" "Trackbar.cpp" "Trackbar.hpp")
target_link_libraries( color-thresholder ${OpenCV_LIBS})
target_compile_features(color-thresholder PRIVATE cxx_range_for)

add_executable(matcher "main-matcher.cpp" "Matcher.cpp" "Matcher.hpp" "Trackbar.cpp")
target_link_libraries( matcher ${OpenCV_LIBS})
target_compile_features(matcher PRIVATE cxx_range_for)
