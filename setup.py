from setuptools import setup, Extension

from os.path import join as path_join

module_folder = "src"

setup(
    name='hello-lib',
    version='1',
    ext_modules=[Extension('_hello', [path_join(module_folder,'_hello.c')])],
)