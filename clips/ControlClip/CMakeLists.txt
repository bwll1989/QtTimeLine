cmake_minimum_required(VERSION 3.16)
project(ControlClip VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 设置源文件
set(SOURCES
    controlclipmodel.hpp
    controlclipplugin.hpp
    ../../AbstractClipModel.hpp
    ../../AbstractClipInterface.h
)

# 创建插件库
add_library(Control MODULE
    ${SOURCES}
)

# 设置目标属性
set_target_properties(Control PROPERTIES
    PREFIX ""
    SUFFIX ".plugin"
)

# 添加编译定义
target_compile_definitions(Control PRIVATE
    NODE_TIMELINE_SHARED
    NODE_TIMELINE_EXPORTS
)

# 链接Qt库
target_link_libraries(Control PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# 包含目录
target_include_directories(Control PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
) 