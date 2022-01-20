cmake_minimum_required(VERSION 3.20)
project(fabric-native-bootstrap)

set(CMAKE_CXX_STANDARD 20)

add_executable(fabric-native-bootstrap src/main.cpp src/Bootstrap.cpp src/Bootstrap.h src/utils/SystemHelper.cpp src/utils/SystemHelper.h src/utils/ISystemHelper.h)

target_link_libraries(fabric-native-bootstrap Shlwapi.lib)