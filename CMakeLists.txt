
cmake_minimum_required(VERSION 3.16)

project(WiFiScope
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp

    src/network/NetworkInterface.cpp
    src/network/ARPScanner.cpp

    src/discovery/HostDiscovery.cpp
    src/scanner/PortScanner.cpp
    src/topology/TopologyBuilder.cpp
    src/topology/TopologyExporter.cpp
)

add_executable(wifiscope ${SOURCES})

target_include_directories(wifiscope
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(wifiscope
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
