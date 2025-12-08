import sys

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'qbaf-py'
copyright = '2023, José Ruiz Alarcón, Timotheus Kampik'
author = 'José Ruiz Alarcón, Timotheus Kampik'
release = '0.0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    'sphinx_design',
    ]

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_book_theme'

html_theme_options = {
    "repository_url": "https://github.com/TimKam/Quantitative-Bipolar-Argumentation",
    "repository_branch": "main",
    "path_to_docs": "doc",

    "use_repository_button": True,
    "use_source_button": False,
    "use_edit_page_button": False,
    "use_download_button": False,
    "use_fullscreen_button": False,

    "logo": {
        # "image_light": "...",
        # "image_dark": "...",
        "text": "QBAF-Py",
        "alt_text": "QBAF-Py documentation",
    }
}
html_static_path = ["_static"]

# --- Path for autodoc ---
sys.path.append('../../')
