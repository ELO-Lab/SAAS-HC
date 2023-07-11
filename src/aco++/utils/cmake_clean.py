import os
import shutil
import argparse
from pathlib import Path

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("acopp_path")

    args = parser.parse_args()
    acopp_path = Path(args.acopp_path)

    remove_list = [
        "build",
        "CMakeFiles",
        "cmake_install.cmake",
        "CMakeCache.txt",
        "Makefile",
    ]
    for item in remove_list:
        file_path = acopp_path / item
        if os.path.isfile(file_path):
            os.remove(file_path)
        if os.path.isdir(file_path):
            shutil.rmtree(file_path)
        print(f"{file_path} is removed")
