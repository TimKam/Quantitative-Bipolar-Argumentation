import pytest
from qbaf import QBAFramework
from qbaf_ctrbs.gradient import determine_gradient_ctrb
from qbaf_ctrbs.removal import determine_removal_ctrb
from qbaf_ctrbs.shapley import determine_shapley_ctrb


def test_cyclic_framework_requires_allow_cycles():
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
    support_relations = []
    # QBAFramework defaults to allow_cycles=False
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model")

    assert framework.isacyclic() is False
    with pytest.raises(NotImplementedError):
        _ = framework.final_strengths


def test_cyclic_framework_converges_when_cycles_are_allowed():
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
    support_relations = []
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)
    final_strengths = framework.final_strengths
    assert framework.isacyclic() is False
    assert framework.allow_cycles is True
    assert final_strengths['a'] == pytest.approx(0.9999999990686774)
    assert final_strengths['b'] == pytest.approx(0.0)
    assert framework.final_strength('a') == pytest.approx(final_strengths['a'])
    assert framework.final_strength('b') == pytest.approx(final_strengths['b'])


def test_cyclic_framework_recomputes_strengths_after_modification():
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
    support_relations = []
    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)
    
    strengths_before_modification = framework.final_strengths
    
    framework.modify_initial_strength('b', 0.8)
    strengths_after_modification = framework.final_strengths
    
    assert strengths_before_modification['a'] == pytest.approx(0.9999999990686774)
    assert strengths_before_modification['b'] == pytest.approx(0.0)
    assert strengths_after_modification['a'] == pytest.approx(1.0)
    assert strengths_after_modification['b'] == pytest.approx(0.0)
    assert strengths_before_modification != strengths_after_modification


def test_contribution_functions_work_for_cyclic_framework():
    arguments = ['a', 'b', 'c']
    initial_strengths = [0.8, 0.4, 0.2]
    attack_relations = [('a', 'b'), ('b', 'a')]
    support_relations = [('c', 'a')]

    framework = QBAFramework(arguments, initial_strengths, attack_relations, support_relations, semantics="DFQuAD_model", allow_cycles=True)

    removal_contribution = determine_removal_ctrb('a', 'c', framework)
    shapley_contribution = determine_shapley_ctrb('a', 'c', framework)
    gradient_contribution = determine_gradient_ctrb('a', 'c', framework)

    assert isinstance(removal_contribution, float)
    assert isinstance(shapley_contribution, float)
    assert isinstance(gradient_contribution, float)


def test_non_converging_cyclic_framework_raises_runtime_error():
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
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
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
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
    arguments = ['a', 'b', 'c']
    initial_strengths = [2.0, 1.0, 1.0]
    attack_relations = [('b', 'a')]
    support_relations = [('c', 'b')]
    # The graph has no cycles but we test with allow_cycles=True
    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="basic_model",
                             allow_cycles=True,)
    assert framework.isacyclic() is True
    assert framework.final_strengths == {'a': 0.0, 'b': 2.0, 'c': 1.0}


def test_reversal_preserves_cycle_settings():
    arguments = ['a', 'b']
    initial_strengths = [1.0, 0.5]
    attack_relations = [('a', 'b'), ('b', 'a')]
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
    assert reversal.final_strengths['a'] == pytest.approx(framework.final_strengths['a'])
    assert reversal.final_strengths['b'] == pytest.approx(framework.final_strengths['b'])


def test_four_node_attack_cycle():
    arguments = ['a', 'b', 'c', 'd']
    initial_strengths = [0.1, 0.1, 0.1, 0.1]
    attack_relations = [('a', 'b'), ('b', 'c'), ('c', 'd'), ('d', 'a')]
    support_relations = []

    framework = QBAFramework(arguments,
                             initial_strengths,
                             attack_relations,
                             support_relations,
                             semantics="DFQuAD_model",
                             allow_cycles=True)

    final_strengths = framework.final_strengths
    assert framework.isacyclic() is False
    assert set(final_strengths.keys()) == set(arguments)
    for argument in arguments:
        assert framework.final_strength(argument) == pytest.approx(final_strengths[argument])
        assert 0.0 <= final_strengths[argument] < 0.1
    values = [final_strengths[arg] for arg in arguments]
    assert values[0] == pytest.approx(values[1])
    assert values[1] == pytest.approx(values[2])
    assert values[2] == pytest.approx(values[3])
