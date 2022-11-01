from setuptools import setup, Extension

from os.path import join as path_join

module_folder = "src"

setup(
    name='qbaf-lib',
    version='0.1',
    ext_modules=[Extension('argument', [path_join(module_folder, 'argument.c')])],
)