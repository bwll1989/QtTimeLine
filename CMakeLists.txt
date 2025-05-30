cmake_minimum_required(VERSION 3.16)

project(QtTimeLine VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用自动 MOC、UIC 和 RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 启用 Windows DLL 导出
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 查找 Qt 包
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS 
    Core
    Widgets 
    Network 
    OpenGL
    OpenGLWidgets
    Gui
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core
    Widgets 
    Network 
    OpenGL
    OpenGLWidgets
    Gui
)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    clips
        clips/ControlClip
        clips/VideoClip
)
set(INCLUDES 
        AbstractClipModel.hpp
        AbstractClipInterface.h
        BaseTimeLineModel.h
        TimeLineStyle.h
        BasePluginLoader.h
        BaseTimeLineToolBar.h
        TimeLineDefines.h
        BaseTimeLineView.h
        BaseTimeLineWidget.h
        BaseTrackDelegate.h
        BaseTrackListView.h
        Export.hpp
        Compiler.hpp
        OperatingSystem.hpp
)
# 源文件列表
set(SOURCES
        BasePluginLoader.cpp
        BaseTimeLineModel.cpp
        BaseTimeLineToolBar.cpp
        BaseTimeLineView.cpp
        BaseTimeLineWidget.cpp
        BaseTrackDelegate.cpp
        BaseTrackListView.cpp
        resources.qrc
)
set(QRC_FILES
        resources.qrc
        )
# 创建共享库
add_library(${PROJECT_NAME} SHARED)

# 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 添加源文件
target_sources(${PROJECT_NAME} PRIVATE
    ${SOURCES}
    ${INCLUDES}
    ${QRC_FILES}
)

# 添加包含目录
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/clips
    ${CMAKE_CURRENT_SOURCE_DIR}/clips/ControlClip
    ${CMAKE_CURRENT_SOURCE_DIR}/clips/VideoClip
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Gui
)

# 如果是 Qt6，添加额外的 OpenGL 依赖
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGL)
endif()

# 添加导出宏定义
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    NODE_TIMELINE_SHARED
    NODE_TIMELINE_EXPORTS
)
# 添加插件子目录
add_subdirectory(clips/ControlClip)
add_subdirectory(clips/VideoClip)
# 添加示例项目
add_subdirectory(example) 

include(GNUInstallDirs)

# 设置安装配置目录
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/QtTimeLine")

# 只安装主库文件
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(FILES ${INCLUDES}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
