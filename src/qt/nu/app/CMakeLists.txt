cmake_minimum_required(VERSION 3.24)

project(DefcoinCoreNuQml VERSION 2026.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Widgets)

qt_add_executable(DefcoinCoreNu
    MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(DefcoinCoreNu
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
)

set_target_properties(DefcoinCoreNu PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.defcoincore.DefcoinCoreNu
    MACOSX_BUNDLE_BUNDLE_NAME "Defcoin Core Nu"
    MACOSX_BUNDLE_DISPLAY_NAME "Defcoin Core Nu"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "2026.1"
    MACOSX_BUNDLE_BUNDLE_VERSION "2026.1.0"
)

target_compile_definitions(DefcoinCoreNu PRIVATE
    DEFCOIN_NU_VERSION="2026.1"
)

file(GLOB_RECURSE DEFCOIN_NU_QML_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../qml/*.qml"
    "${CMAKE_CURRENT_LIST_DIR}/../qml/qmldir"
)

file(GLOB_RECURSE DEFCOIN_NU_ASSET_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../assets/*"
)

add_custom_target(DefcoinCoreNuResources ALL
    DEPENDS ${DEFCOIN_NU_QML_FILES} ${DEFCOIN_NU_ASSET_FILES}
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_BUNDLE_DIR:DefcoinCoreNu>/Contents/Resources/nu"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:DefcoinCoreNu>/Contents/Resources/nu"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../qml" "$<TARGET_BUNDLE_DIR:DefcoinCoreNu>/Contents/Resources/nu/qml"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../assets" "$<TARGET_BUNDLE_DIR:DefcoinCoreNu>/Contents/Resources/nu/assets"
)

add_dependencies(DefcoinCoreNuResources DefcoinCoreNu)
