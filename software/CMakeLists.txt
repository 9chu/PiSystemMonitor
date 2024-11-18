cmake_minimum_required(VERSION 3.10)

################################################################################
# 包管理器
################################################################################

# <editor-fold desc="CPM 包管理器">

# CPM 包管理器可以辅助从 github 源下载依赖并导入项目
set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_DOWNLOAD_SHA256 "c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION}
            EXPECTED_HASH SHA256=${CPM_DOWNLOAD_SHA256})
endif()

include(${CPM_DOWNLOAD_LOCATION})

# </editor-fold>
# <editor-fold desc="VCPKG 包管理器">

# 使用 CPM 包管理器从 github 源引入 vcpkg
CPMAddPackage(
    NAME vcpkg
    GITHUB_REPOSITORY microsoft/vcpkg
    GIT_TAG 2024.07.12
    DOWNLOAD_ONLY)

if(${vcpkg_ADDED})
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# </editor-fold>

################################################################################
# 工程配置
################################################################################

# 为了正确与 vcpkg 协作，所有第三方依赖必须晚于 project 载入
project(PiSystemMonitor C CXX)

# <editor-fold desc="全局编译选项">

# 全局编译选项，影响第三方库
set(CMAKE_CXX_STANDARD 20)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /wd5030 /Zc:preprocessor")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /Zc:preprocessor")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-attributes")
endif()

# </editor-fold>
# <editor-fold desc="其他第三方依赖">

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.91.5
    DOWNLOAD_ONLY)

if (NOT TARGET imgui AND imgui_ADDED)
    add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp)
    target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})

    add_executable(binary_to_compressed_c ${imgui_SOURCE_DIR}/misc/fonts/binary_to_compressed_c.cpp)
endif ()

CPMAddPackage(
    NAME implot
    GITHUB_REPOSITORY epezent/implot
    GIT_TAG v0.16
    DOWNLOAD_ONLY)

if (NOT TARGET implot AND implot_ADDED)
    add_library(implot STATIC ${implot_SOURCE_DIR}/implot.cpp ${implot_SOURCE_DIR}/implot_items.cpp)
    target_include_directories(implot INTERFACE ${implot_SOURCE_DIR})
    target_link_libraries(implot PUBLIC imgui)
endif ()

# </editor-fold>
# <editor-fold desc="代码">

find_package(sdl2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ada CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(double-conversion CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE_FILES "include/*.hpp" "src/*.cpp")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/whitrabt.ttf.inl
    COMMAND $<TARGET_FILE:binary_to_compressed_c> ${CMAKE_CURRENT_SOURCE_DIR}/assets/whitrabt.ttf kFontWhitrabt >
        ${CMAKE_CURRENT_BINARY_DIR}/whitrabt.ttf.inl
    DEPENDS binary_to_compressed_c ${CMAKE_CURRENT_SOURCE_DIR}/assets/whitrabt.ttf)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Segment7-4Gml.otf.inl
    COMMAND $<TARGET_FILE:binary_to_compressed_c> ${CMAKE_CURRENT_SOURCE_DIR}/assets/Segment7-4Gml.otf kFontSegment7 >
        ${CMAKE_CURRENT_BINARY_DIR}/Segment7-4Gml.otf.inl
    DEPENDS binary_to_compressed_c ${CMAKE_CURRENT_SOURCE_DIR}/assets/Segment7-4Gml.otf)

add_executable(PiSystemMonitor ${SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/whitrabt.ttf.inl
    ${CMAKE_CURRENT_BINARY_DIR}/Segment7-4Gml.otf.inl)
target_include_directories(PiSystemMonitor PRIVATE include ${OPENGL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(PiSystemMonitor PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    imgui implot fmt::fmt spdlog::spdlog ${OPENGL_LIBRARIES} httplib::httplib
    unofficial::concurrentqueue::concurrentqueue ada::ada double-conversion::double-conversion)

# </editor-fold>
