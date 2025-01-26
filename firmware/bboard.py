import argparse
from bb_py.bb_setup import BBBuildSetup
from bb_py.bb_builder import BBBuilder
from bb_py.bb_cleaner import BBCleaner


def add_args(parser):
    parser.add_argument("-bs", "--buildsetup", action="store_true",
                        help="Run build setup.")
    parser.add_argument("-b", "--build", action="store_true",
                        help="Run build.")
    parser.add_argument("-clean", "--clean", action="store_true",
                        help="Run clean.")
    
    BBBuildSetup.add_args(parser)


def execute_command(args):
    if args.buildsetup:
        [build_type, build_tests] = BBBuildSetup.parse_args(args)
        bb_setup = BBBuildSetup()
        bb_setup.run_build_setup(build_type, build_tests)
    elif args.build:
        bb_builder = BBBuilder()
        bb_builder.run_build()
    elif args.clean:
        bb_cleaner = BBCleaner()
        bb_cleaner.run_clean()


def main():
    parser = argparse.ArgumentParser()

    add_args(parser)

    args = parser.parse_args()

    execute_command(args)


if __name__ == "__main__":
    main()
