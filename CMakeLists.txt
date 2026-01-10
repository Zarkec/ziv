cmake_minimum_required(VERSION 3.19)
project(ziv LANGUAGES CXX)

# 设置OpenCV路径
set(OpenCV_DIR "D:/Project/opencv-mingw/x64/mingw/lib")

# 查找OpenCV包
find_package(OpenCV REQUIRED)

# 查找Qt包
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)

qt_standard_project_setup()

qt_add_executable(ziv
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/core/imagegraphicsview.cpp
    src/core/imagegraphicsview.h
    src/core/imageviewer.cpp
    src/core/imageviewer.h
    src/core/measurementtool.cpp
    src/core/measurementtool.h
)

target_link_libraries(ziv
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Concurrent
        ${OpenCV_LIBS}
)

# 添加OpenCV头文件路径
target_include_directories(ziv PRIVATE ${OpenCV_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(GNUInstallDirs)

install(TARGETS ziv
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ziv
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
