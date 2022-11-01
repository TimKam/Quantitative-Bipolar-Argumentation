from setuptools import setup, Extension
import os

def main():
    # Folder that contains the header files
    include_folder = "include"
    # Folder that contains the C code
    module_folder = "src"
    # Files in module folder
    source_files = [os.path.join(module_folder, file) for file in os.listdir(module_folder)]

    setup(
        name='qbaf-lib',
        version='0.1',
        ext_modules=[Extension('qbaf', 
                        include_dirs = [include_folder],
                        sources = source_files)],
    )

if __name__ == '__main__':
    main()