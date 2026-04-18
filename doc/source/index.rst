QBAF-Py Documentation
===================================
QBAF-Py is a library for drawing inferences from Quantitative Bipolar Argumentation Frameworks (QBAFs) and explaining them.
The library is written in CPython (C with a Python API) to facilitate speed and efficiency.

To use QBAF-Py, the main object to create is a ``QBAFramework``. At a high level, a framework is built from four pieces:

- the arguments in the graph,
- the initial strength of each argument,
- the attack relations,
- the support relations.

So the user describes which arguments exist, how strong they are initially, and how they influence one another.

For example, a small framework can be created like this:

.. code-block:: python

   from qbaf import QBAFramework

   qbaf = QBAFramework(
       ['a', 'b', 'c'],
       [0.8, 0.4, 0.7],
       [('b', 'a')],
       [('c', 'a')],
       semantics="DFQuAD_model",
   )

Here, ``b`` attacks ``a``, ``c`` supports ``a``, and ``semantics`` specifies how those interactions are turned into final strengths.

The supported semantics are:

- Basic model: ``"basic_model"``
- Quadratic Energy: ``"QuadraticEnergy_model"``
- Squared DF-QuAD: ``"SquaredDFQuAD_model"``
- Euler-Based Top: ``"EulerBasedTop_model"``
- Euler-Based: ``"EulerBased_model"``
- DF-QuAD: ``"DFQuAD_model"``

Once the framework has been created, you can inspect whether it is acyclic and what final strengths it assigns:

.. code-block:: python

   qbaf.final_strengths
   qbaf.final_strength('a')



To understand how different arguments affect a topic argument, QBAF-Py provides several contribution functions, supporting both individual contributors and sets of contributors.

The available contribution functions are:

- Removal contribution: ``determine_removal_ctrb``
- Intrinsic removal contribution: ``determine_iremoval_ctrb``
- Shapley contribution: ``determine_shapley_ctrb``
- Partitioned Shapley contribution: ``determine_partitioned_shapley_ctrb``
- Gradient contribution: ``determine_gradient_ctrb``

For example, to compute the removal contribution of ``c`` to ``a``:

.. code-block:: python

   from qbaf_ctrbs.removal import determine_removal_ctrb

   contribution = determine_removal_ctrb('a', 'c', qbaf)
   print(contribution)

Cyclic Frameworks
#################

QBAF-Py also supports cyclic frameworks.
A framework can contain cycles through its attack and support relations, and you can check this with ``qbaf.isacyclic()``.

By default, cycle handling is disabled.
This means that if the framework is cyclic, accessing ``final_strengths`` raises an error unless cycle support is enabled explicitly.
To evaluate a cyclic framework, you can enable cycle support by passing ``allow_cycles=True``.
To control the convergence criteria, you can also pass the following arguments when constructing it:

- ``max_iterations`` (default ``1000``)
- ``convergence_threshold`` (default ``1e-9``)

For example:

.. code-block:: python

   cyclic_qbaf = QBAFramework(
       ['a', 'b'],
       [1.0, 0.5],
       [('a', 'b'), ('b', 'a')],
       [],
       semantics="DFQuAD_model",
       allow_cycles=True,
       max_iterations=1000,
       convergence_threshold=1e-9,
   )

   cyclic_qbaf.isacyclic()
   cyclic_qbaf.final_strengths

When cycle support is enabled, QBAF-Py computes final strengths by synchronous fixed-point iteration.
It starts from the initial strengths, updates all arguments from the previous iteration, and stops once the change is below the convergence threshold.
If convergence is not reached within ``max_iterations``, a ``RuntimeError`` is raised.

.. note::

   Whether a cyclic framework converges or not can depend on the chosen semantics.

The cycle-related settings are exposed on the framework instance through:

- ``allow_cycles`` (``bool``)
- ``max_iterations`` (``int``)
- ``convergence_threshold`` (``float``)











Contribution Functions
######################

   .. autofunction:: qbaf_ctrbs.removal.determine_removal_ctrb
   .. autofunction:: qbaf_ctrbs.intrinsic_removal.determine_iremoval_ctrb
   .. autofunction:: qbaf_ctrbs.shapley.determine_shapley_ctrb
   .. autofunction:: qbaf_ctrbs.shapley.determine_partitioned_shapley_ctrb
   .. autofunction:: qbaf_ctrbs.gradient.determine_gradient_ctrb
   .. autofunction:: qbaf_ctrbs.utils.restrict


Visualization Support
#####################

   .. autofunction:: qbaf_visualizer.Visualizer.visualize



Core API
########

.. automodule:: qbaf
   :members:



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
