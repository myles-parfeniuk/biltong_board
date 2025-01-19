import os
import sys
import subprocess

def main():
    # Set default for BUILD_TESTS to OFF
    build_tests = "OFF"

    if len(sys.argv) > 1 and sys.argv[1] == "--tests":
        build_tests = "ON"

    print(f"Building with tests: {build_tests}")

    script_dir = os.path.dirname(os.path.realpath(__file__))
    
    toolchain_file = "gcc-arm-none-eabi.cmake"
    toolchain_path = os.path.join(script_dir, toolchain_file)

    if not os.path.isfile(toolchain_path):
        print(f"ERROR: {toolchain_file} not found in the current directory.")
        sys.exit(1)

    print(f"Using toolchain: {toolchain_path}")

    cmake_command = [
        "cmake",
        "-Bbuild",
        "-G", "Ninja",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_path}",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=true",
        f"-DBUILD_TESTS={build_tests}"
    ]

    try:
        subprocess.run(cmake_command, check=True)
        print("CMake configuration completed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error running cmake: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
