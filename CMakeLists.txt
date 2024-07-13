cmake_minimum_required(VERSION 3.28)

project(huffman_codec)

set(CMAKE_CXX_STANDARD 23)


# Set project dirs
set(IMGUI_DIR external/imgui)
set(SFD_DIR external/sfd/src)
set(TESTS_DIR tests)

# BUILD OPTIONS
set(CMAKE_BUILD_TYPE "Release")
set(ARGUMENTUM_BUILD_STATIC_LIBS ON)
set(CMAKE_GENERATOR "Ninja")


# OPTION TO SELECT CLI/GUI APP BUILD
option(HUFFMANCODEC_BUILD_GUI "Build the huffman_codec GUI variant" OFF)

if (HUFFMANCODEC_BUILD_GUI)
    add_definitions(-DHUFFMANCODEC_GUI)
endif ()

include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
include_directories(${SFD_DIR})

add_subdirectory(external/argumentum)
add_subdirectory(src/lib)

# GTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

set(EXEC_SOURCES
    # IMGUI Sources
    ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp
    # IMGUI Backends
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
    ${SFD_DIR}/sfd.cpp
    # Exec
    src/main.cpp
)

# Build-type dependant executables
if (HUFFMANCODEC_BUILD_GUI)
    add_executable(huffman_codec
        ${EXEC_SOURCES}
        src/gui/ui_dx12.cpp
    )
else ()
    add_executable(huffman_codec
        ${EXEC_SOURCES}
        src/cli/cmd.cpp
    )
endif()

add_executable(huffman_test
        ${TESTS_DIR}/huffman_tree_test.cc
        ${TESTS_DIR}/huffman_codec_test.cc
)

# Lib links
target_link_libraries(huffman_codec huffman_lib)
if (HUFFMANCODEC_BUILD_GUI)
    # DirectX XII Libs
    target_link_libraries(huffman_codec d3d12.lib dwmapi.lib d3dcompiler.lib dxgi.lib comdlg32.lib)
else ()
    target_link_libraries(huffman_codec Argumentum::argumentum)
endif()


# Include dirs
target_include_directories(huffman_codec PUBLIC src/lib)
target_include_directories(huffman_test PUBLIC src/lib)

target_link_libraries(huffman_test GTest::gtest_main huffman_lib)

include(GoogleTest)
gtest_discover_tests(huffman_test)
