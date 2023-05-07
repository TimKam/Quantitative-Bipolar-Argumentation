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
        name='QBAF-Py',
        python_requires='>=3.8',
        version='0.1.0',
        description='QBAF-Py is a library for drawing inferences from Quantitative Bipolar Argumentation Frameworks (QBAFs) and explaining them.',
        author='José Ruiz Alarcón, Timotheus Kampik',
        py_modules=['qbaf_visualizer'],
        ext_modules=[Extension('qbaf', 
                        include_dirs = [include_folder],
                        sources = source_files)],
        extras_require={
            'dev': [
                'pytest'
            ],
            'visualizer': [
                'networkx',
                'pygraphviz'    
            ]
        },
        license='GPL-2'
    )

if __name__ == '__main__':
    main()