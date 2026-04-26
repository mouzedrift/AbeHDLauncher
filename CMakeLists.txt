cmake_minimum_required(VERSION 3.16)

project(AbeHDLauncher VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

set(zip_src
    3rdParty/zip/miniz.h
    3rdParty/zip/zip.h
    3rdParty/zip/zip.c
)

set(abe_hd_launcher_src
    src/main.cpp
    src/mainwindow.ui
    src/mainwindow.hpp
    src/mainwindow.cpp
    src/downloader.hpp
    src/downloader.cpp
    src/assetfile.hpp
    src/resources.qrc
    src/assetsdialog.ui
    src/assetsdialog.hpp
    src/assetsdialog.cpp
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/src/icon.rc")

qt_add_executable(AbeHDLauncher ${zip_src} ${abe_hd_launcher_src} ${app_icon_resource_windows})

target_include_directories(AbeHDLauncher PRIVATE 3rdParty)

target_link_libraries(AbeHDLauncher PRIVATE Qt6::Widgets Qt6::Network dwmapi)

set_target_properties(AbeHDLauncher PROPERTIES
    WIN32_EXECUTABLE ON
)
