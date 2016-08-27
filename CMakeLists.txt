cmake_minimum_required(VERSION 3.0)
project(LudumDare36)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SFML_GameEngine/src)
    message(FATAL_ERROR "SFML_GameEngine is missing!\nPlease update the SFML_GameEngine submodule by running 'git submodule init' and 'git submodule update'!")
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/EntityComponentMetaSystem/src)
    message(FATAL_ERROR "EntityComponentMetaSystem is missing!\nPlease update the EntityComponentMetaSystem submodule by running 'git submodule init' and 'git submodule update'!")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SFML_GameEngine/src SFML_GameEngineBuild)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EntityComponentMetaSystem/src EntityComponentMetaSystemBuild)

set(LudumDare36_SOURCES
    src/screens/GameScreen.cpp
    src/main.cpp)

add_executable(LudumDare36 ${LudumDare36_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D NDEBUG")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug', none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

target_include_directories(LudumDare36 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/SFML_GameEngine/src ${CMAKE_CURRENT_SOURCE_DIR}/EntityComponentMetaSystem/src)
target_link_libraries(LudumDare36 SFML_GameEngine)

