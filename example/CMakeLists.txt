cmake_minimum_required(VERSION 3.5)

project(TimelineExample VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用自动 MOC、UIC 和 RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt 包
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core OpenGL OpenGLWidgets Gui )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core OpenGL OpenGLWidgets Gui)

# 源文件
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 链接 Qtimeline 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    QtTimeLine
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Gui
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
