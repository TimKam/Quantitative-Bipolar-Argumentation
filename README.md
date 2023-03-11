# QBAF-Py
![CI status](https://github.com/TimKam/Quantitative-Bipolar-Argumentation/actions/workflows/actions.yml/badge.svg)

This project provides _QBAF-Py_, a library for drawing inferences from Quantitative Bipolar Argumentation Frameworks (QBAFs) and explaining them.
The library is written in CPython (C with a Python API) to facilitate speed and efficiency.

## Requirements & Installation
Installing *QBAF-Py* requires [pip](https://pypi.org/project/pip/).

To install *QBAF-Py*, run in the project's root directory:

```
pip install .
```

Note: To install on Windows, Microsoft Visual C++ 14.0 or greater might be required.

## Dependencies
*QBAF-Py* does not have any dependencies!
Only if you want to work on the *QBAF-Py* code base, you should install some *dev dependencies* for testing.

## Tutorials
To illustrate how *QBAF-Py* works, the notebook [examples.ipynb](https://github.com/TimKam/Quantitative-Bipolar-Argumentation/blob/main/examples.ipynb) has several examples available.
The notebook is also available on [Google Colab](https://colab.research.google.com/drive/1EDOpBgmHZmZtAJx3N5wOeXVuwMaTWf8M?usp=sharing).

## Documentation
Generated documentation in html can be found [here](https://htmlpreview.github.io/?https://github.com/TimKam/Quantitative-Bipolar-Argumentation/blob/gh-pages/index.html).

You can also generate documentation in other formats making use of sphinx (you need to have sphinx installed).

The documentation is genereted from your installed libraries, so you need to install *QBAF-Py* first.

To generate the documentation, run in the [doc](https://github.com/TimKam/Quantitative-Bipolar-Argumentation/tree/main/doc) directory:

```
make
```

You will be able to see all the formats for the documentation.
For example, if you want html documentation run:

```
make html
```

## Testing
To test the package locally, first install the test requirements (`pip install -e .[dev]`) and then run `pytest` in the project's root directory.

## Acknowledgements
**Authors**: José Ruiz Alarcón - [@Ruiz968](https://github.com/Ruiz968), Timotheus Kampik - [@TimKam](https://github.com/TimKam)

This project is mainly based on the paper called *Explaining Change in Quantitative Bipolar Argumentation* (Kampik & Čyras, [2022](https://ebooks.iospress.nl/doi/10.3233/FAIA220152)).