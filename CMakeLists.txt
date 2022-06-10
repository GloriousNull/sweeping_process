cmake_minimum_required(VERSION 3.20)
project(sweeping_process)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_RELEASE "/cgthreads8 /std:c++20 /permissive- /O2")

add_library(project_warnings INTERFACE)
include(cmake/compiler-warnings.cmake)
set_project_warnings(project_warnings)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "lib/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML COMPONENTS graphics audio REQUIRED)

#SET(IMGUI_DIR "~/lib/imgui")
#SET(IMGUI_SFML_DIR "~/lib/imgui-sfml")
SET(IMGUI_DIR "lib/imgui-master")
SET(IMGUI_SFML_DIR "lib/imgui-sfml-master")

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_SFML_DIR})

file(GLOB_RECURSE LOCAL_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
list(FILTER LOCAL_SOURCES EXCLUDE REGEX "lib/*")

set(SOURCES ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_SFML_DIR}/imgui-SFML.cpp)

add_executable(sweeping_process ${SOURCES} ${LOCAL_SOURCES})


target_link_libraries(sweeping_process sfml-graphics sfml-audio)
target_compile_options(sweeping_process INTERFACE ${PROJECT_WARNINGS})
