cmake_minimum_required(VERSION 3.19)
project(OnionKnight LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(OnionKnight
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/app/main_window/mainwindow.cpp
    src/app/main_window/mainwindow.h

    src/app/work_field/workfield.h
    src/app/work_field/work_field.cpp

    src/app/work_field_view/workfieldview.h
    src/app/work_field_view/workfieldview.cpp
)

target_include_directories(OnionKnight
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/main_window
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/work_field
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/work_field_view
)

target_link_libraries(OnionKnight
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS OnionKnight
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET OnionKnight
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
