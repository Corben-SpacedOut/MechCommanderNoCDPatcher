@echo off
echo ===================================
echo Building MechCommanderNoCDPatch...
echo ===================================

:: Create build directory if it doesn't exist
if not exist build mkdir build

:: Navigate to build directory
cd build

:: Configure with CMake if CMakeCache.txt doesn't exist
if not exist CMakeCache.txt (
    echo Configuring CMake project...
    cmake .. || goto :error
)

:: Build the project
echo Building project...
cmake --build . --config Debug || goto :error

:: If we get here, build was successful
echo.
echo ===================================
echo Build successful!
echo ===================================
echo.
echo Running MechCommanderNoCDPatch...
echo.

:: Run the executable
.\bin\Debug\MechCommanderNoCDPatch.exe

:: Return to original directory
cd ..

goto :end

:error
echo.
echo ===================================
echo Error: Build failed!
echo ===================================
cd ..
exit /b 1

:end
exit /b 0
