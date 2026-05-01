cmake_minimum_required(VERSION 3.19)
project(OnionKnight LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

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
    scripts/component_inspector.py
    src/app/component/meta/componentmeta.h
    src/app/component/meta/componentmeta.cpp
    src/app/component/inspector/componentinspector.h
    src/app/component/inspector/componentinspector.cpp
    src/app/component/registry/componentregistry.h
    src/app/component/registry/componentregistry.cpp
    src/app/component/port/componentport.h
    src/app/component/port/componentport.cpp
    src/app/component/item/componentitem.h
    src/app/component/item/componentitem.cpp
    src/app/component/tree_model/componenttreemodel.h
    src/app/component/tree_model/componenttreemodel.cpp
    src/app/component/filter_proxy_model/componentfilterproxymodel.h
    src/app/component/filter_proxy_model/componentfilterproxymodel.cpp
    src/app/component/tab_widget/componenttabwidget.h
    src/app/component/tab_widget/componenttabwidget.cpp
    src/app/tab_panel/tabpanel.h
    src/app/tab_panel/tabpanel.cpp
)

target_include_directories(OnionKnight
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/main_window
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/work_field
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/work_field_view
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/meta
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/inspector
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/registry
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/port
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/item
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/tree_model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/filter_proxy_model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/component/tab_widget
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app/tab_panel
)

target_link_libraries(OnionKnight
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
)

add_custom_command(
    TARGET OnionKnight POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/component_inspector.py
        $<TARGET_FILE_DIR:OnionKnight>/scripts/component_inspector.py
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
