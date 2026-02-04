cmake_minimum_required(VERSION 3.16)

project(gmk67_ui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(hidapi REQUIRED)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/Backend/HidManager.cpp
    src/Backend/HidManager.h
    src/Backend/ProtocolController.cpp
    src/Backend/ProtocolController.h
    src/Backend/ProtocolController.h
)

qt_add_executable(gmk67_control
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(gmk67_control
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    hidapi::hidapi
)

# Resources
qt_add_qml_module(gmk67_control
    URI gmk67_ui
    VERSION 1.0
    QML_FILES
        src/UI/Main.qml
        src/UI/LightingView.qml
        src/UI/KeymapView.qml
        src/UI/Layout.js
        src/UI/FullLayout.js
)

install(TARGETS gmk67_control
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
