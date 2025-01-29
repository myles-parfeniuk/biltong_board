import argparse
import os
import sys
import time
import subprocess
import re
import shutil
from enum import IntEnum, auto


class BBUtPath:
    @staticmethod
    def get_project_dir():
        try:
            path_project = os.path.dirname(
                os.path.dirname(os.path.realpath(__file__)))
        except Exception as e:
            BBUtLog.log(f"ERROR while getting the project directory: {e}")
            sys.exit(1)

        return path_project

    @staticmethod
    def get_build_dir():
        try:
            path_build = os.path.join(BBUtPath.get_project_dir(), "build")
        except Exception as e:
            BBUtLog.log(f"ERROR while getting the project directory: {e}")
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
            BBUtLog.log(f"ERROR while getting toolchain path: {e}")
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
        
class BBUtCodeScrubber:

    class LineType(IntEnum):
        CODE = auto()
        SINGLE_LINE_COMMENT = auto()
        COMMENT_BLOCK_START = auto()
        COMMENT_BLOCK_END = auto()
        EMPTY = auto()

    @staticmethod
    def read_file(path_file):
        lines = None
        with open(path_file, 'r') as file:
            lines = file.readlines()
        
        return lines

    @staticmethod
    def identify_line_type(line):
        line = line.strip()

        if re.match(r'^\s*/\*.*\*/\s*$', line):
            return BBUtCodeScrubber.LineType.SINGLE_LINE_COMMENT
        elif re.match(r'^\s*//', line): 
            return BBUtCodeScrubber.LineType.SINGLE_LINE_COMMENT
        elif re.match(r'^\s*\*/\s*$', line): 
            return BBUtCodeScrubber.LineType.COMMENT_BLOCK_END
        elif re.match(r'/\*+', line):
            return BBUtCodeScrubber.LineType.COMMENT_BLOCK_START
        elif line == "":
            return BBUtCodeScrubber.LineType.EMPTY
        else:
            return BBUtCodeScrubber.LineType.CODE

    @staticmethod
    def find_fxn(path_file, fxn_name):
        lines = BBUtCodeScrubber.read_file(path_file)
        content = ''.join(lines)

        fxn_start_pattern = rf'\w+\s+{re.escape(fxn_name)}\s*\([^)]*\)\s*{{'
        fxn_start = re.search(fxn_start_pattern, content)

        if fxn_start:
            start = fxn_start.start()

            # find the eof via counting closing braces (this could break but should be okay with stm32cube code)
            brace_count = 1
            end = start
            for i in range(start + fxn_start.end() - fxn_start.start(), len(content)):
                if content[i] == '{':
                    brace_count += 1
                elif content[i] == '}':
                    brace_count -= 1
                    if brace_count == 0:
                        end = i + 1
                        break

            start_line = content[:start].count('\n') + 1
            end_line = content[:end].count('\n') + 1

            return start_line, end_line
        else:
            BBUtLog.log(f"Function '{fxn_name}' not found in '{path_file}'.")
            return None, None

    @staticmethod
    def find_comment_block(path_file, fxn_name, fxn_start):
        lines = BBUtCodeScrubber.read_file(path_file)

        comment_start = None
        comment_end = fxn_start - 2
        in_comment_block = False
        line_type = BBUtCodeScrubber.identify_line_type(lines[comment_end])

        if (line_type != BBUtCodeScrubber.LineType.CODE):
            in_comment_block = True
            i = comment_end

            while (i >= 0) and (i < len(lines)):
                i -= 1
                line = lines[i]
                line_type = BBUtCodeScrubber.identify_line_type(line)

                if in_comment_block:
                    if (line_type == BBUtCodeScrubber.LineType.SINGLE_LINE_COMMENT) or (line_type == BBUtCodeScrubber.LineType.COMMENT_BLOCK_START):
                        in_comment_block = False
                else:
                    if (line_type == BBUtCodeScrubber.LineType.SINGLE_LINE_COMMENT) or (line_type == BBUtCodeScrubber.LineType.COMMENT_BLOCK_END):
                        in_comment_block = True
                    elif (line_type == BBUtCodeScrubber.LineType.CODE):
                        comment_start = i + 1
                        break

                if line_type == BBUtCodeScrubber.LineType.EMPTY:
                    comment_start = i + 1
                    break

            if (comment_start != None) and (comment_end != None):
                if comment_end >= comment_start:
                    return comment_start, (comment_end + 1)

        BBUtLog.log(f"No comment block found for '{fxn_name} for in '{path_file}'")
        return None, None

    @staticmethod
    def find_references(path_file, fxn_name):
        references = []

        lines = BBUtCodeScrubber.read_file(path_file)

        # find references
        for line_number, line in enumerate(lines, 1):
            if fxn_name in line and ';' in line:
                reference = line
                references.append((line_number, reference))
                #find comment block for respective reference
                comment_start, comment_end = BBUtCodeScrubber.find_comment_block(path_file, f"{path_file}#{line_number}L: reference", line_number)
                if (comment_start != None) and (comment_end != None):
                    for i in range(comment_start, comment_end + 1):
                        references.append((i + 1, lines[i]))

        if references:   
            #sort to according to line number (ascending)                        
            references.sort(key=lambda x: x[0])
            return references
        else:
            BBUtLog.log(f"No references to '{fxn_name}' found in '{path_file}'.")
            return None

    @staticmethod
    def delete_lines(path_file, lines_to_delete):
        lines = BBUtCodeScrubber.read_file(path_file)

        deleted_lines = []
        with open(path_file, 'w') as file:
            for i, line in enumerate(lines, 1):
                if i in lines_to_delete:
                    deleted_lines.append((i, line))
                else:
                    file.write(line)

        for reference in deleted_lines:
            BBUtLog.log(f"{path_file}#{reference[0]}L: Deleted: {reference[1]}")

    @staticmethod
    def cleanup_references(path_file, reference_name):
        references = BBUtCodeScrubber.find_references(
            path_file, reference_name)

        if references != None:
            BBUtLog.log(
                f"Scrubbing '{reference_name} references from '{path_file}'...")
            BBUtLog.log("---------------------------------------------------------------------")
            lines_to_delete = {reference[0] for reference in references}

            BBUtCodeScrubber.delete_lines(path_file, lines_to_delete)

    @staticmethod
    def cleanup_fxn(path_file, fxn_name):
        [fxn_start, fxn_end] = BBUtCodeScrubber.find_fxn(path_file, fxn_name)
        if fxn_start != None:
            [comment_start, comment_end] = BBUtCodeScrubber.find_comment_block(
                path_file, fxn_name, fxn_start)

        if fxn_start != None:
            BBUtLog.log(
                f"Scrubbing '{fxn_name} definition and header from '{path_file}'...")
            BBUtLog.log("---------------------------------------------------------------------")

            if comment_start != None:
                lines_to_delete = list(
                    range(fxn_start, fxn_end + 1)) + list(range(comment_start, comment_end + 1))
            else:
                lines_to_delete = list(range(fxn_start, fxn_end + 1))

            BBUtCodeScrubber.delete_lines(path_file, lines_to_delete)

        BBUtCodeScrubber.cleanup_references(path_file, fxn_name)
