cmake_minimum_required(VERSION 3.16)
project(TextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    PrintSupport
    Network
    Concurrent
)

# Find backtrace package
find_package(Backtrace REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/editor.cpp
    src/sessionmanager.cpp
    src/filemanager.cpp
    src/dialogs/finddialog.cpp
    src/dialogs/settingsdialog.cpp
    src/dialogs/recoverydialog.cpp
    src/dialogs/autocorrectdialog.cpp
    src/syntax/syntaxhighlighter.cpp
    src/splitviewcontainer.cpp
    src/toolbar.cpp
    src/contextmenu.cpp
    src/crashhandler.cpp
)

# Header files
set(HEADERS
    include/mainwindow.h
    include/editor.h
    include/sessionmanager.h
    include/filemanager.h
    include/dialogs/finddialog.h
    include/dialogs/settingsdialog.h
    include/dialogs/recoverydialog.h
    include/dialogs/autocorrectdialog.h
    include/syntax/syntaxhighlighter.h
    include/splitviewcontainer.h
    include/toolbar.h
    include/contextmenu.h
    include/crashhandler.h
)

# UI files
set(UI_FILES
    forms/mainwindow.ui
    forms/dialogs/finddialog.ui
    forms/dialogs/settingsdialog.ui
    forms/dialogs/recoverydialog.ui
    forms/dialogs/autocorrectdialog.ui
)

# Resource files
set(RESOURCE_FILES
    resources/resources.qrc
)

# Create executable
add_executable(TextEditor
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Include directories
target_include_directories(TextEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6PrintSupport_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Concurrent_INCLUDE_DIRS}
)

# Link Qt libraries
target_link_libraries(TextEditor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Network
    Qt6::Concurrent
    ${Backtrace_LIBRARIES}
)

# Add compile definitions
target_compile_definitions(TextEditor PRIVATE
    $<$<CONFIG:Debug>:QT_DEBUG>
    $<$<CONFIG:Release>:QT_NO_DEBUG>
)

# Install rules
install(TARGETS TextEditor
    RUNTIME DESTINATION bin
)
