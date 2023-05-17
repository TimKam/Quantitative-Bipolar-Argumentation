.. qbaf-py documentation master file, created by
   sphinx-quickstart on Fri Dec  2 20:07:06 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to QBAF-Py's documentation!
===================================
QBAF-Py is a library for drawing inferences from Quantitative Bipolar Argumentation Frameworks (QBAFs) and explaining them. The library is written in CPython (C with a Python API) to facilitate speed and efficiency.

Core API
########

.. automodule:: qbaf
   :members:

Contribution Functions
######################

   .. autofunction:: qbaf_ctrbs.removal.determine_removal_ctrb
   .. autofunction:: qbaf_ctrbs.intrinsic_removal.determine_iremoval_ctrb
   .. autofunction:: qbaf_ctrbs.shapley.determine_shapley_ctrb
   .. autofunction:: qbaf_ctrbs.utils.restrict

Visualization Support
#####################

   .. autofunction:: qbaf_visualizer.Visualizer.visualize

