cmake_minimum_required(VERSION 3.16)

project(GhostClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 and fail if it's not found
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Multimedia)

# DBus is used on Linux to inhibit idle suspend / screen blanking during playback
if(NOT WIN32)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
endif()

# Project sources
set(PROJECT_SOURCES
    main.cpp
    Medium.cpp
    Medium.h
    Navigator.cpp
    Navigator.h
    VLCPlayerHandler.cpp
    VLCPlayerHandler.h
    qml.qrc
    resources.qrc
    conf.ini # Added to track changes in IDE
)

# Include the Windows resource file for the app icon
if(WIN32)
    list(APPEND PROJECT_SOURCES GhostClient.rc)
endif()

# Create executable (WIN32 flag hides the console window on Windows)
if(WIN32)
    add_executable(GhostClient WIN32 ${PROJECT_SOURCES})
else()
    add_executable(GhostClient ${PROJECT_SOURCES})
endif()

# Add current directory to include path so <Navigator.h> works
target_include_directories(GhostClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Pass the source directory to C++ so it can locate conf.ini during development without copying
target_compile_definitions(GhostClient PRIVATE "PROJECT_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

# Link Qt modules
target_link_libraries(GhostClient PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Multimedia
)

# Windows-specific libraries and VLC setup
if(WIN32)
    # Link Windows Sockets API (used in main.cpp for DNS resolution)
    target_link_libraries(GhostClient PRIVATE ws2_32)

    # Determine architecture for local VLC package
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VLC_ARCH "x64")
    else()
        set(VLC_ARCH "x86")
    endif()

    set(VLC_PKG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/VideoLAN.LibVLC.Windows.3.0.21/build/${VLC_ARCH}")

    # Include VLC headers from the local NuGet package
    target_include_directories(GhostClient PRIVATE "${VLC_PKG_DIR}/include")

    # Find VLC library (.lib for MSVC or .dll.a for MinGW)
    find_library(VLC_LIBRARY
        NAMES vlc libvlc
        PATHS "${VLC_PKG_DIR}/lib"
        NO_DEFAULT_PATH
    )

    if(VLC_LIBRARY)
        target_link_libraries(GhostClient PRIVATE ${VLC_LIBRARY})
    else()
        message(WARNING "VLC library not found in ${VLC_PKG_DIR}/lib")
    endif()
else()
    # Native Linux build (if you ever need it)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(VLC REQUIRED libvlc)
    target_include_directories(GhostClient PRIVATE ${VLC_INCLUDE_DIRS})
    target_link_libraries(GhostClient PRIVATE ${VLC_LIBRARIES} Qt6::DBus)
endif()
