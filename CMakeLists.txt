cmake_minimum_required(VERSION 3.28)

set(CMAKE_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

project(chess)

file(GLOB_RECURSE SRCS "src/**")
add_executable(chess ${SRCS})

if (EMSCRIPTEN) 
    target_compile_options(chess PRIVATE "-sUSE_GLFW=3")
    target_link_options(chess PRIVATE "-sUSE_GLFW=3")
    target_link_options(chess PRIVATE "-sUSE_WEBGL2=1")
    target_link_options(chess PRIVATE "-sFULL_ES3=1")
    target_link_options(chess PRIVATE "-sASYNCIFY")
    target_link_options(chess PRIVATE "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets")
    set_target_properties(chess PROPERTIES SUFFIX ".html")
endif()

include(FetchContent)

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG "2c980bb59875b0d32144a71867fbdebb2f77cd20"
)
FetchContent_MakeAvailable(stb)

if (NOT EMSCRIPTEN)
    # GLFW
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG "3.5.1"
    )
    FetchContent_MakeAvailable(glfw)

    # GLAD
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        658f48e72aee3c6582e80b05ac0f8787a64fe6bb
        SOURCE_SUBDIR  cmake
    )
    FetchContent_MakeAvailable(glad)
    glad_add_library(glad_gl_core_43 STATIC REPRODUCIBLE LOADER API gl:core=4.3)
endif()

target_link_libraries(chess PUBLIC glfw)
target_include_directories(chess PUBLIC ${stb_SOURCE_DIR})

if (NOT EMSCRIPTEN)
    target_link_libraries(chess PUBLIC glad_gl_core_43)
endif()

