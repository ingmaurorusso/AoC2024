
# set Day to get correct file names.
set(DAY 04)

#CMake minimum version
cmake_minimum_required(VERSION 3.5.0)

#C++ Standard version
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fconcepts-diagnostics-depth=2)

project(AOC_2024_Day${DAY} LANGUAGES CXX) # VERSION 0.1.0 )

set(TARGET_EXE ${PROJECT_NAME})
#message("PROJECT_NAME = ${PROJECT_NAME}")
#message("TARGET_EXE = ${TARGET_EXE}")


add_definitions(-DDAY=${DAY})
add_definitions(-DINPUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
#message("INPUT_PATH = ${DINPUT_PATH}")

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=1000000")

add_executable(
    ${TARGET_EXE} Day${DAY}_main.cpp Day${DAY}_solution_part_1.cpp Day${DAY}_solution_part_2.cpp
)

target_include_directories(${TARGET_EXE} PRIVATE ../include/)
