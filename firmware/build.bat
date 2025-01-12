@echo off

:: Set default for BUILD_TESTS to OFF
set BUILD_TESTS=OFF

:: Check if the user provided the BUILD_TESTS argument
if "%1"=="--tests" (
    set BUILD_TESTS=ON
)

:: Display the chosen option
echo Building with tests: %BUILD_TESTS%

:: Read the toolchain file path from toolchain_path.txt
set TOOLCHAIN_PATH=

:: Check if the toolchain_path.txt file exists in the current directory
if exist toolchain_path.txt (
    set /p TOOLCHAIN_PATH=<toolchain_path.txt
) else (
    echo ERROR: toolchain_path.txt not found in the current directory.
    exit /b 1
)

:: Display the toolchain path that was read
echo Using toolchain: %TOOLCHAIN_PATH%

:: Run the cmake command to configure the project
cmake -Bbuild -G Ninja -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_PATH% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=true -DBUILD_TESTS=%BUILD_TESTS%
