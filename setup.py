from setuptools import setup, Extension
import os

def main():
    # Folder that contains all the C code
    module_folder = "src"
    # Relative paths of the files in module folder
    files = [os.path.join(module_folder, file) for file in os.listdir(module_folder)]

    setup(
        name='qbaf-lib',
        version='0.1',
        ext_modules=[Extension('qbaf', files)],
    )

if __name__ == '__main__':
    main()