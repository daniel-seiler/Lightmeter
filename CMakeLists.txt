cmake_minimum_required(VERSION 3.0.0)
set(PRJ_NAME "lightmeter")
project(${PRJ_NAME} CXX)

include(FindPythonInterp)
set (py_cmd "scripts/generate_header.py")
execute_process(
        COMMAND ${PYTHON_EXECUTABLE} ${py_cmd}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_executable(${PRJ_NAME} ${SOURCE_FILES})

include_directories("${PROJECT_SOURCE_DIR}/libs/Adafruit_BusIO")
target_link_arduino_libraries(${PRJ_NAME} AUTO_PUBLIC)
target_enable_arduino_upload(${PRJ_NAME})
