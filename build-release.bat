@echo off
echo ===================================
echo Building MechCommanderNoCDPatch RELEASE version...
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

:: Build the project in Release mode
echo Building project in Release mode...
cmake --build . --config Release || goto :error

:: If we get here, build was successful
echo.
echo ===================================
echo Build successful!
echo ===================================
echo.

:: Copy the executable to the project root
echo Copying executable to project root...
copy .\bin\Release\MechCommanderNoCDPatch.exe ..\ || goto :copy_error
echo.
echo Release executable copied to project root.

:: Return to original directory
cd ..

goto :end

:copy_error
echo.
echo ===================================
echo Error: Failed to copy executable!
echo ===================================
cd ..
exit /b 2

:error
echo.
echo ===================================
echo Error: Build failed!
echo ===================================
cd ..
exit /b 1

:end
echo.
echo ===================================
echo Release build complete!
echo ===================================
echo.
exit /b 0
