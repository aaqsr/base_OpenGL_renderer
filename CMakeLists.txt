# After reconfiguring any aspect of CMake project: cmake --preset debug
#   Runs configuration: sets up build/debug dir, generates build files, etc.

# Actually building or rebuilding: cmake --build --preset debug

cmake_minimum_required(VERSION 3.19)

project(main)

message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES
        src/*.cpp
        src/*/*.cpp)

# Symbolic link for media directory. May not work on windows?
# file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/media"
# "${CMAKE_CURRENT_BINARY_DIR}/media" SYMBOLIC)

add_executable(main
        ${SRC_FILES})

# Headers
target_include_directories(main
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

# Link libs

find_package(OpenGL REQUIRED)

## GLFW3
find_package(glfw3 CONFIG REQUIRED)

message(STATUS "GLFW3 found: ${glfw3_FOUND}")
message(STATUS "GLFW3 version: ${glfw3_VERSION}")
get_target_property(GLFW_LOCATION glfw LOCATION)
message(STATUS "GLFW library location: ${GLFW_LOCATION}")

target_link_libraries(main PRIVATE glfw)

## GLM
find_package(glm CONFIG REQUIRED)

message(STATUS "GLM found: ${glm_FOUND}")
message(STATUS "GLM version: ${glm_VERSION}")

target_link_libraries(main PRIVATE glm::glm)

## GLEW
find_package(GLEW REQUIRED)

message(STATUS "GLEW found: ${GLEW_FOUND}")
message(STATUS "GLEW version: ${GLEW_VERSION}")
message(STATUS "GLEW include dirs: ${GLEW_INCLUDE_DIRS}")
message(STATUS "GLEW libraries: ${GLEW_LIBRARIES}")
get_target_property(GLEW_LOCATION GLEW::GLEW LOCATION)
message(STATUS "GLEW library location: ${GLEW_LOCATION}")

target_link_libraries(main PRIVATE GLEW::GLEW)

## IMGUI
find_package(imgui CONFIG REQUIRED)
target_link_libraries(main PRIVATE imgui::imgui)

# Generate compile_commands.json in the build directory
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(APPLE)
    target_compile_definitions(main PRIVATE GL_SILENCE_DEPRECATION)

    # Link required frameworks
    target_link_libraries(main PRIVATE
        OpenGL::GL                    # OpenGL framework
        "-framework Cocoa"            # Cocoa framework
        "-framework IOKit"            # IOKit framework
        "-framework CoreFoundation"   # CoreFoundation framework
    )

    message(STATUS "OpenGL::GL target available: ${OpenGL_GL_FOUND}")
    message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
endif()

if (WIN32)
    # On Windows, copy directories instead of creating symbolic links
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders"
            COMMENT "copying shaders to build directory"
    )

    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets"
            COMMENT "copying assets to build directory"
    )

    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/scenes"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/scenes"
            COMMENT "copying scenes to build directory"
    )

    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "${CMAKE_BINARY_DIR}/shaders"
            COMMENT "copying shaders to build directory"
    )

    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "${CMAKE_BINARY_DIR}/assets"
            COMMENT "copying assets to build directory"
    )

    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/scenes"
            "${CMAKE_BINARY_DIR}/scenes"
            COMMENT "copying scenes to build directory"
    )
else ()
    # use symbolic links
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/shaders" SYMBOLIC)
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets" SYMBOLIC)
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/scenes" "${CMAKE_BINARY_DIR}/scenes" SYMBOLIC)
    # If we generated the compile commands, then copy or symlink compile_commands.json to the project root (only if it's not already there)
    if (CMAKE_EXPORT_COMPILE_COMMANDS)
        add_custom_command(TARGET main POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_BINARY_DIR}/compile_commands.json"
                    "${CMAKE_SOURCE_DIR}/compile_commands.json"
            )
    endif ()
endif ()

# MS BUILD YUM YUM WOO
# TODO: NO CLUE whether this works...
if (WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
    # set startup project for Visual Studio
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

    # set working directory for debugging in Visual Studio
    set_target_properties(main PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif ()

# platform based compiler flags
if (MSVC)
    # why oh why microsoft are these not just gcc/clang flags sigh
    target_compile_options(main PRIVATE
            /W4  # Warning level 4
            $<$<CONFIG:Debug>:/Od /Zi>  # Debug: no optimization, debug info
            $<$<CONFIG:Release>:/O2>    # Release: optimize for speed
    )

    # define DEBUG for debug builds
    target_compile_definitions(main PRIVATE
            $<$<CONFIG:Debug>:DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
    )
else ()
    # gcc/clang flags
    target_compile_options(main PRIVATE
            -Wall -Wextra
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3>
    )

    # define DEBUG for debug builds
    target_compile_definitions(main PRIVATE
            $<$<CONFIG:Debug>:DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
    )
endif ()

# Documentation with Doxygen.
# Taken from https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/docs/compiled/Doxyfile.in)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
