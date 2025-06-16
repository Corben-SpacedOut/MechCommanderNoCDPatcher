cmake_minimum_required(VERSION 3.10)

# Project name
project(MechCommanderNoCDPatch VERSION 1.0 LANGUAGES C)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Windows specific settings
if(WIN32)
  # Set minimum Windows version to Windows 10
  add_definitions(-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00)
  
  # Add WIN32 flag to create a GUI application (no console window)
  add_executable(MechCommanderNoCDPatch  src/main.c src/patch.c)
  
  # Set Windows 10 as minimum OS version in PE header
  # This will make Windows show an error dialog if run on older versions
  # Windows 10 is version 10.0 (10.0.0.0)
  if(MSVC)
    # For MSVC, use the subsystem version properties file
    set_target_properties(MechCommanderNoCDPatch PROPERTIES
      VS_USER_PROPS "${CMAKE_CURRENT_SOURCE_DIR}/win10_support.props")
  endif()
  
  # Add the minimum OS version using linker flags directly
  # This works with both MSVC and other compilers like MinGW
  if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,10.00 /MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/win10_compat.manifest")
  else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--major-os-version=10,--minor-os-version=0,--major-subsystem-version=10,--minor-subsystem-version=0")
  endif()
  
  # Add src directory as include directory
  target_include_directories(MechCommanderNoCDPatch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

  # Link with required Windows libraries
  target_link_libraries(MechCommanderNoCDPatch user32 gdi32 comctl32 comdlg32)
endif()
