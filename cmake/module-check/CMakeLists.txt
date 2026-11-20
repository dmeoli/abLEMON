# A module whose template walks a std::vector by its iterators, and a unit
# that imports it and instantiates the template without including <vector>,
# as one that imports the module lemon does: it builds where the generator
# and the compiler build modules whose templates find the operators of the
# standard library [see LEMON_BUILD_MODULE in the main CMakeLists.txt]
CMAKE_MINIMUM_REQUIRED(VERSION 3.28)
PROJECT(lemon_module_check CXX)
SET(CMAKE_CXX_STANDARD 20)
ADD_LIBRARY(m)
TARGET_SOURCES(m PUBLIC FILE_SET CXX_MODULES FILES m.cppm)
ADD_EXECUTABLE(u u.cpp)
TARGET_LINK_LIBRARIES(u PRIVATE m)
