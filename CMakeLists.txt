cmake_minimum_required(VERSION 3.21.1)
project(palyer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(./src ./ui D:/Program\ Files/ffmpeg/ffmpeg-5.0.1-full_build-shared/include)

file(GLOB ALL_UI_HEAD_FILE RELATIVE F:/Clion_project/palyer/ ./ui/*.h )
file(GLOB ALL_UI_SRC_FILE RELATIVE F:/Clion_project/palyer/ ./ui/*.cpp )
file(GLOB ALL_HEAD_FILE RELATIVE F:/Clion_project/palyer/ ./src/*.cpp )
file(GLOB ALL_SRC_FILE RELATIVE F:/Clion_project/palyer/ ./src/*.h )

file(GLOB ALL_UI_FILE RELATIVE F:/Clion_project/palyer/ ./ui/*.ui )

#aux_source_directory(src ALL_SRC_FILE)

set(CMAKE_PREFIX_PATH "F:/Qt/Qt5.12.12/5.12.12/mingw73_64")

link_directories(D:/lib/ffmpeg/lib)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        REQUIRED)

add_executable(palyer
        ${ALL_HEAD_FILE}
        ${ALL_SRC_FILE}
        ${ALL_UI_HEAD_FILE}
        ${ALL_UI_SRC_FILE}
        ${ALL_UI_FILE}
        ${ALL_SRC_FILE}
        )
target_link_libraries(palyer
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Multimedia
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        )

if (WIN32)
    set(DEBUG_SUFFIX)
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
