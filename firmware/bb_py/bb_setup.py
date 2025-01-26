import argparse
import os
import shutil
import sys
import time
import subprocess
from bb_py.bb_utility import BBUtCli, BBUtPath, BBUtLog


class BBBuildSetup:
    def __init__(self):
        self.path_build = None
        self.path_toolchain = None
        self.build_type = None
        self.build_tests = None

    @staticmethod
    def add_args(parser):
        parser.add_argument("-r", "--release", action="store_true",
                            help="BuildSetup: Configure build as release mode, default is debug.")
        parser.add_argument("-t", "--tests", action="store_true",
                            help="BuildSetup: Configure build with tests.")

    @staticmethod
    def parse_args(args):
        build_type = "Release" if args.release else "Debug"
        build_tests = "ON" if args.tests else "OFF"

        return [build_type, build_tests]

    def wipe_build_dir(self):
        BBUtLog.log("Wiping build directory contents...")

        try:
            shutil.rmtree(self.path_build)
            os.makedirs(self.path_build)
        except:
            BBUtLog.log(f"ERROR while wiping the build directory...")
            sys.exit(1)

        BBUtLog.log("Build directory contents wiped...")

    def display_build_options(self):
        BBUtLog.log(f"Configuring build for: {self.build_type}...")
        BBUtLog.log(f"Configuring build with tests: {self.build_tests}...")

        BBUtLog.log("Starting build...")

    def create_cmake_command(self):
        BBUtLog.log("Creating cmake command...")
        cmake_command = [
            "cmake",
            "-Bbuild",
            "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={self.path_toolchain}",
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=true",
            f"-DBUILD_TESTS={self.build_tests}"
        ]

        BBUtLog.log("CMake command created successfully...:")

        BBUtLog.log(
            "     ----------------------------------------------------------")
        for item in cmake_command:
            BBUtLog.log(f"    {item}")
        BBUtLog.log(
            "     ---------------------------------------------------------")

        return cmake_command

    def run_cmake_command(self, cmake_command):
        try:
            BBUtLog.log("Running CMake configuration...")
            BBUtCli.run_cli_cmd(cmake_command)
            BBUtLog.log("Make configuration completed successfully...")
        except subprocess.CalledProcessError as e:
            print(f"ERROR running CMake configuration: {e.returncode}")
            sys.exit(1)

    def run_build_setup(self, build_type="Debug", build_tests="OFF"):
        BBUtLog.reset_timer()
        BBUtLog.log("######################## BBoard Build Config Begin ########################")
        self.build_type = build_type
        self.build_tests = build_tests
        self.path_build = BBUtPath.get_build_dir()
        self.wipe_build_dir()
        self.display_build_options()
        BBUtLog.log("Searching for 'gcc-arm-none-eabi.cmake'...")
        self.path_toolchain = BBUtPath.get_toolchain_path()
        BBUtLog.log(f"Found '{self.path_toolchain}'...")
        cmake_command = self.create_cmake_command()
        self.run_cmake_command(cmake_command)
        BBUtLog.log("######################## BBoard Build Config Fin ########################")
