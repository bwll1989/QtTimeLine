cmake_minimum_required(VERSION 3.16)
project(VideoClip VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/Timeline)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/Timeline)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 设置源文件
set(SOURCES
    videoclipmodel.hpp
    videoclipplugin.hpp
    ../../AbstractClipModel.hpp
    ../../AbstractClipInterface.h
)

# 创建插件库
add_library(Video MODULE
    ${SOURCES}
)

# 设置目标属性
set_target_properties(Video PROPERTIES
    PREFIX ""
    SUFFIX ".plugin"
)

# 链接Qt库
target_link_libraries(Video PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
        QtTimeLine
)

# 包含目录
target_include_directories(Video PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
) 

# 添加编译定义
target_compile_definitions(Video PRIVATE
    NODE_TIMELINE_SHARED
    NODE_TIMELINE_EXPORTS
)