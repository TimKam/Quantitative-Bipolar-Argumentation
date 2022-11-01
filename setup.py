from setuptools import setup, Extension

from os.path import join as path_join

module_folder = "src"

setup(
    name='test-qbaf-test',
    version='1',
    ext_modules=[Extension('_custom', [path_join(module_folder, '_custom.c')])],
)