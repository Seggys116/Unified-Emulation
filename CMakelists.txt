# Cmake version
cmake_minimum_required(VERSION 3.20)

# Global Compiler Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -static")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Allow external libraries
include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

# TODO: Build External Projects First

# Open-GL setups
ExternalProject_Add(OpenGL # Using 4.6 So no macOS support RN (They dont really like OpenGl anyways)
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND  ""
)

# SOIL2 setups
ExternalProject_Add(SOIL2
    GIT_REPOSITORY https://github.com/SpartanJ/SOIL2.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND  ""
)

# GLM OpenGL Mathematics
ExternalProject_Add(GLM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND  ""
)

# FreeType2 font library
ExternalProject_Add(FreeType2
    GIT_REPOSITORY https://github.com/freetype/freetype
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND  ""
)

# FreeType2 font library
ExternalProject_Add(OpenAL
    GIT_REPOSITORY https://github.com/kcat/openal-soft.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DLIBTYPE=STATIC
    UPDATE_COMMAND  ""
)

# Main project definition
project(emulation-game)

#For Threading Find and use
find_package (Threads REQUIRED)
add_compile_options()

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c*")

add_executable(Emulator ${SOURCES})
target_include_directories(Emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/build/external/include/freetype2)
add_dependencies(Emulator OpenGL SOIL2 GLM FreeType2 OpenAL)
target_link_libraries(Emulator PRIVATE stdc++ freetype soil2 glfw3 ${OPENGL_LIBRARIES} OpenAL32 winmm.lib ${CMAKE_THREAD_LIBS_INIT}) # winmm.lib may be windows only!