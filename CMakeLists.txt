cmake_minimum_required(VERSION 3.16)

project(ziv VERSION 1.0.0 LANGUAGES CXX)

# -------------------------------------------------
# 基础 CMake / Qt 自动化配置
# -------------------------------------------------
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UI 文件搜索路径（AUTOUIC）
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/core
)

# -------------------------------------------------
# OpenCV
# -------------------------------------------------
set(OpenCV_DIR "D:/Project/opencv-mingw/x64/mingw/lib")
find_package(OpenCV REQUIRED)

# -------------------------------------------------
# Qt（优先 Qt6，兼容 Qt5）
# -------------------------------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Concurrent)

message(STATUS "Found Qt version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")

# -------------------------------------------------
# 源码组织
# -------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(CORE_SOURCES
    ${SRC_DIR}/core/imagegraphicsview.cpp
    ${SRC_DIR}/core/imageviewer.cpp
    ${SRC_DIR}/core/measurementtool.cpp
    ${SRC_DIR}/core/anglemeasurementtool.cpp
)

set(CORE_HEADERS
    ${SRC_DIR}/core/imagegraphicsview.h
    ${SRC_DIR}/core/imageviewer.h
    ${SRC_DIR}/core/measurementtool.h
    ${SRC_DIR}/core/anglemeasurementtool.h
)

set(UI_SOURCES
    ${SRC_DIR}/ui/mainwindow.cpp
)

set(UI_HEADERS
    ${SRC_DIR}/ui/mainwindow.h
)

set(MAIN_SOURCES
    ${SRC_DIR}/main.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${MAIN_SOURCES}
)

# -------------------------------------------------
# 资源文件
# -------------------------------------------------
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(RESOURCES
    ${RESOURCES_DIR}/common.qrc
)

# -------------------------------------------------
# Windows 资源文件（用于设置 exe 图标）
# -------------------------------------------------
if(WIN32)
    set(WIN_RESOURCES ${RESOURCES_DIR}/app.rc)
endif()

# -------------------------------------------------
# 可执行目标
# -------------------------------------------------

qt_add_executable(ziv
    WIN32 MACOSX_BUNDLE
    MANUAL_FINALIZATION
    ${ALL_SOURCES}
    ${RESOURCES}
    ${WIN_RESOURCES}
)

# -------------------------------------------------
# 头文件搜索路径
# -------------------------------------------------
target_include_directories(ziv PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${SRC_DIR}
    ${SRC_DIR}/core
    ${SRC_DIR}/ui
    ${CMAKE_BINARY_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

# -------------------------------------------------
# 链接库
# -------------------------------------------------
target_link_libraries(ziv PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${OpenCV_LIBS}
)

# -------------------------------------------------
# Qt6 finalize（必须）
# -------------------------------------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(ziv)
endif()

# -------------------------------------------------
# 安装 & windeployqt 自动部署（Windows）
# -------------------------------------------------
include(GNUInstallDirs)

install(TARGETS ziv
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(WIN32 AND QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_generate_deploy_app_script(
        TARGET ziv
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
