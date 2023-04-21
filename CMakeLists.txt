cmake_minimum_required(VERSION 3.5)

project(Flasher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    LinguistTools
    Network
    SerialPort
)

set(TS_FILES Flasher_en_GB.ts)

set(MACOSX_BUNDLE_ICON_FILE raisedev.icns)
set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/raisedev.icns")
set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set(PROJECT_SOURCES
    main.cpp
    flasher.cpp
    flasher.h
    flasher.ui
    ${TS_FILES}
    ${APP_ICON_MACOS}
)

qt_add_executable(Flasher
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(Flasher PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
)

set_target_properties(Flasher PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Flasher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(MSVC)
  target_compile_options(Flasher PRIVATE /W4 /WX)
else()
  target_compile_options(Flasher PRIVATE -Wall -Wextra -Werror)
endif()

qt_finalize_executable(Flasher)
