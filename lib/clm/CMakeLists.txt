cmake_minimum_required (VERSION 3.8)

project ("clm" C)
set(CMAKE_BUILD_TYPE Release)

include_directories(include)
add_library("clm" src/clm.c)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries("clm" PUBLIC ${MATH_LIBRARY})
endif()