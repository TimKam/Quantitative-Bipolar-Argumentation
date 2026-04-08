import pytest
from qbaf import QBAFramework
from qbaf_ctrbs.gradient import determine_gradient_ctrb
from qbaf_ctrbs.removal import determine_removal_ctrb
from qbaf_ctrbs.shapley import determine_shapley_ctrb


def test_cyclic_framework_requires_allow_cycles():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    # QBAFramework defaults to allow_cycles=False
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model")

    assert framework.isacyclic() is False
    with pytest.raises(NotImplementedError):
        _ = framework.final_strengths


def test_cyclic_framework_converges_when_cycles_are_allowed():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)
    final_strengths = framework.final_strengths
    assert framework.isacyclic() is False
    assert framework.allow_cycles is True
    assert final_strengths["A"] == pytest.approx(0.9999999990686774)
    assert final_strengths["B"] == pytest.approx(0.0)
    assert framework.final_strength("A") == pytest.approx(final_strengths["A"])
    assert framework.final_strength("B") == pytest.approx(final_strengths["B"])


def test_cyclic_framework_recomputes_strengths_after_modification():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)
    
    strengths_before_modification = framework.final_strengths
    
    framework.modify_initial_strength("B", 0.8)
    strengths_after_modification = framework.final_strengths
    
    assert strengths_before_modification["A"] == pytest.approx(0.9999999990686774)
    assert strengths_before_modification["B"] == pytest.approx(0.0)
    assert strengths_after_modification["A"] == pytest.approx(1.0)
    assert strengths_after_modification["B"] == pytest.approx(0.0)
    assert strengths_before_modification != strengths_after_modification


def test_contribution_functions_work_for_cyclic_framework():
    arguments = ["A", "B", "C"]
    initial_strengths = [0.8, 0.4, 0.2]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = [("C", "A")]

    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)

    removal_contribution = determine_removal_ctrb("A", "C", framework)
    shapley_contribution = determine_shapley_ctrb("A", "C", framework)
    gradient_contribution = determine_gradient_ctrb("A", "C", framework)

    assert isinstance(removal_contribution, float)
    assert isinstance(shapley_contribution, float)
    assert isinstance(gradient_contribution, float)


def test_non_converging_cyclic_framework_raises_runtime_error():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="basic_model",
                             allow_cycles=True,
                             max_iterations=1000,
                             convergence_threshold=1e-9,)
    with pytest.raises(RuntimeError, match="did not converge"):
        _ = framework.final_strengths


def test_cycle_settings_are_exposed():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="DFQuAD_model",
                             allow_cycles=True,
                             max_iterations=17,
                             convergence_threshold=1e-5,)
    assert framework.allow_cycles is True
    assert framework.max_iterations == 17
    assert framework.convergence_threshold == pytest.approx(1e-5)


def test_allow_cycles_does_not_change_acyclic_framework_behavior():
    arguments = ["A", "B", "C"]
    initial_strengths = [2.0, 1.0, 1.0]
    attack_relations = [("B", "A")]
    support_relations = [("C", "B")]
    # The graph has no cycles but we test with allow_cycles=True
    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="basic_model",
                             allow_cycles=True,)
    assert framework.isacyclic() is True
    assert framework.final_strengths == {"A": 0.0, "B": 2.0, "C": 1.0}


def test_reversal_preserves_cycle_settings():
    arguments = ["A", "B"]
    initial_strengths = [1.0, 0.5]
    attack_relations = [("A", "B"), ("B", "A")]
    support_relations = []
    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="DFQuAD_model",
                             allow_cycles=True,
                             max_iterations=850,
                             convergence_threshold=1e-5,)

    reversal = framework.reversal(framework, [])
    assert reversal.allow_cycles is True
    assert reversal.max_iterations == 850
    assert reversal.convergence_threshold == pytest.approx(1e-5)
    assert reversal.final_strengths["A"] == pytest.approx(framework.final_strengths["A"])
    assert reversal.final_strengths["B"] == pytest.approx(framework.final_strengths["B"])
