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
   .. autofunction:: qbaf_ctrbs.shapley.determine_partitioned_shapley_ctrb
   .. autofunction:: qbaf_ctrbs.gradient.determine_gradient_ctrb
   .. autofunction:: qbaf_ctrbs.utils.restrict

Robustness Functions
####################
   .. autofunction:: qbaf_robustness.consistency_checks.is_general_robust_consistent
   .. autofunction:: qbaf_robustness.consistency_checks.is_expansion_robust_consistent
   .. autofunction:: qbaf_robustness.consistency_checks.is_bounded_updates_robust_consistent
   .. autofunction:: qbaf_robustness.consistency_checks.is_bounded_update
   .. autofunction:: qbaf_robustness.consistency_checks.is_expansion
   .. autofunction:: qbaf_robustness.inconsistency_checks.is_general_robust_inconsistent
   .. autofunction:: qbaf_robustness.inconsistency_checks.is_expansion_robust_inconsistent
   .. autofunction:: qbaf_robustness.inconsistency_checks.is_bounded_updates_robust_inconsistent
   .. autofunction:: qbaf_robustness.explanations.new_arguments
   .. autofunction:: qbaf_robustness.explanations.is_pocket
   .. autofunction:: qbaf_robustness.explanations.pockets_of_consistency
   .. autofunction:: qbaf_robustness.explanations.explanation_of_robustness_violation
   
Visualization Support
#####################

   .. autofunction:: qbaf_visualizer.Visualizer.visualize

