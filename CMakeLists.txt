cmake_minimum_required(VERSION 3.2)
project( FetchMyBeer )

set(CMAKE_PREFIX_PATH "/usr/local/opt/opencv3")
include_directories(${CMAKE_PREFIX_PATH}/include)
find_package(OpenCV REQUIRED)
# include_directories(/usr/local/opt/opencv3/include)
# link_directories(/usr/local/opt/opencv3/lib)
add_subdirectory(src)
