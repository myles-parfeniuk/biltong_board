import argparse
import os
import sys
import time
import subprocess



class BBUtPath:
    @staticmethod
    def get_project_dir():
        try:
            path_project = os.path.dirname(
                os.path.dirname(os.path.realpath(__file__)))
        except Exception as e:
            print(f"ERROR while getting the project directory: {e}")
            sys.exit(1)

        return path_project

    @staticmethod
    def get_build_dir():
        try:
            path_build = os.path.join(BBUtPath.get_project_dir(), "build")
        except Exception as e:
            print(f"ERROR while getting the project directory: {e}")
            sys.exit(1)

        return path_build

    def get_toolchain_path():
        toolchain_file = "gcc-arm-none-eabi.cmake"
        toolchain_path = os.path.join(BBUtPath.get_project_dir(), toolchain_file)

        try:
            if not os.path.isfile(toolchain_path):
                raise FileNotFoundError(
                    f"'{toolchain_file}' not found in project dir.")

        except Exception as e:
            print(f"ERROR while getting toolchain path: {e}")
            sys.exit(1)

        return toolchain_path
    
class BBUtLog:
    start_time = 0

    @staticmethod
    def reset_timer():
        BBUtLog.start_time = time.perf_counter()

    @staticmethod
    def log(msg):
        time_stamp_ms = 1000 * (time.perf_counter() - BBUtLog.start_time)
        print(f"[{int(time_stamp_ms):010d}]ms: {msg}")


class BBUtCli:
    @staticmethod
    def run_cli_cmd(command):
        process = subprocess.Popen(
            command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

        for line in process.stdout:
            BBUtLog.log(f"{line.strip()}")
            sys.stdout.flush()

        process.wait()
        if process.returncode != 0:
            raise subprocess.CalledProcessError(process.returncode, command)
