# QBAF-Py
![CI status](https://github.com/TimKam/Quantitative-Bipolar-Argumentation/actions/workflows/actions.yml/badge.svg)

This project provides _QBAF-Py_, a library for drawing inferences from Quantitative Bipolar Argumentation Frameworks (QBAFs) and explaining them.
The library is written in CPython (C with a Python API) to facilitate speed and efficiency.

In order to use QBAF-Py, run the following command in the root directory of this project:

pip install .

Build requirements for Windows (installation and usage merely requires Python): 
- Microsoft Visual C++ 14.0 or greater

## Testing
To test the package locally, first install the test requirements (`pip install -e .[dev]`) and then run `pytest` in the project's root directory.