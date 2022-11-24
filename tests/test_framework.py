import pytest
from qbaf import QBAFramework, QBAFARelations

# TEST INIT

def test_init_correct_args():
    args = ['a', 'b', 'c']
    initial_weights = [1, 1, 5]
    att = [('a', 'c')]
    supp = [('a', 'b')]
    QBAFramework(args, initial_weights, att, supp,
        disjoint_relations=True,
        semantics="basic_model",
        aggregation_function=None,
        influence_function=None,
        min_weight=-1.7976931348623157e+308,
        max_weight=1.7976931348623157e+308)

def test_init_semantics():
    args,initial_weights,att,supp = ['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')]
    QBAFramework(args, initial_weights, att, supp, semantics="basic_model")
    QBAFramework(args, initial_weights, att, supp, semantics="QuadraticEnergy_model")
    QBAFramework(args, initial_weights, att, supp, semantics="SquaredDFQuAD_model")
    QBAFramework(args, initial_weights, att, supp, semantics="EulerBasedTop_model")
    QBAFramework(args, initial_weights, att, supp, semantics="EulerBased_model")
    QBAFramework(args,[0.1, 0.1, 0.5], att, supp, semantics="DFQuAD_model")
    with pytest.raises(ValueError):
        QBAFramework(args, initial_weights, att, supp, semantics="incorrect_model")

def test_init_no_args():
    with pytest.raises(TypeError):
        QBAFramework()

def test_init_args_initial_weights():
    QBAFramework(['a', 'b'], [1, 1], [], [])
    with pytest.raises(TypeError):
        QBAFramework(['a', 'b'], [1, "1"], [], [])
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1], [], [])
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1, 1, 5], [], [])
    with pytest.raises(TypeError):
        QBAFramework({'a', 'b'}, [1, 1], [], [])
    with pytest.raises(TypeError):
        QBAFramework(['a', 'b'], {1, 1}, [], [])

def test_init_relations():
    QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    QBAFramework(['a', 'b', 'c'], [1, 1, 5], {('a', 'c')}, {('a', 'b')})
    with pytest.raises(TypeError):
        QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b', 'b')])
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1, 1], [('a', 'c')], [('a', 'b')])
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1, 1], [('a', 'b')], [('a', 'c')])

def test_init_disjoint_relations():
    QBAFramework(['a', 'b'], [1, 1], [('a', 'b')], [('a', 'b')], disjoint_relations=False)
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1, 1], [('a', 'b')], [('a', 'b')], disjoint_relations=True)
    with pytest.raises(ValueError):
        QBAFramework(['a', 'b'], [1, 1], [('a', 'b')], [('a', 'b')])

def test_init_getters():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=True)
    assert qbf.arguments == {'a', 'b', 'c'}
    assert qbf.initial_weights == {'a':1.0, 'b':1.0, 'c':5.0}
    assert qbf.attack_relations.relations == {('a', 'c')}
    assert qbf.support_relations.relations == {('a', 'b')}
    assert qbf.disjoint_relations == True
    assert qbf.final_weights == {'a': 1.0, 'c': 4.0, 'b': 2.0}
    assert qbf.semantics == "basic_model"

def test_init_setters():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=True)
    with pytest.raises(AttributeError):
        qbf.arguments = set()
    with pytest.raises(AttributeError):
        qbf.initial_weights = dict()
    with pytest.raises(AttributeError):
        qbf.attack_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.support_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.final_weights = dict()
    with pytest.raises(TypeError):
        qbf.disjoint_relations = 0
    qbf.disjoint_relations = False
    assert qbf.disjoint_relations == False
    qbf.add_support_relation('a', 'c')
    with pytest.raises(ValueError):
        qbf.disjoint_relations = True

# TEST ARGUMENTS

def test_modify_arguments():
    qbf = QBAFramework(['a', 'b', 'c', 'd'], [1, 1, 5, 3], [('a', 'c')], [('a', 'b')])
    assert qbf.arguments == {'a', 'b', 'c', 'd'}
    qbf.remove_argument('d')
    assert qbf.arguments == {'a', 'b', 'c'}
    qbf.add_argument('d', 1)
    assert qbf.arguments == {'a', 'b', 'c', 'd'}
    assert qbf.initial_weights['d'] == 1.0
    with pytest.raises(ValueError):
        qbf.remove_argument('a')
    with pytest.raises(ValueError):
        qbf.remove_argument('b')
    with pytest.raises(ValueError):
        qbf.remove_argument('c')
    qbf.remove_argument('d')
    assert qbf.contains_argument('a') and qbf.contains_argument('b') and qbf.contains_argument('c')
    assert not qbf.contains_argument('d')

# TEST INITIAL WEIGHTS

def test_access_initial_weight():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    for argument, weight in qbf.initial_weights.items():
        assert qbf.initial_weight(argument) == weight
    with pytest.raises(ValueError):
        qbf.initial_weight('d')

def test_modify_initial_weight():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    initial_weights = qbf.initial_weights
    initial_weights['a'] = 0.0
    assert qbf.initial_weight('a') == 1.0

    qbf.modify_initial_weight('c', 4)
    assert qbf.initial_weight('c') == 4.0

    qbf.add_argument('a', 0.0)
    assert qbf.initial_weight('a') == 1.0

# TEST ATTACK RELATIONS

def test_access_attack_relations():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    assert qbf.attack_relations.relations == {('a', 'c')}
    assert qbf.contains_attack_relation('a', 'c')
    assert not qbf.contains_attack_relation('c', 'a')
    assert not qbf.contains_attack_relation('a', 'b')
    for a, b in [('a', 'c'), ('a', 'b'), ('c', 'a'), ('b', 'a')]:
        assert qbf.contains_attack_relation(a,b) == qbf.attack_relations.contains(a, b)
        assert qbf.contains_attack_relation(a,b) == ((a,b) in qbf.attack_relations)

def test_modify_attack_relations():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbf.remove_attack_relation('a', 'c')
    assert ('a', 'c') not in qbf.attack_relations
    qbf.add_attack_relation('a', 'c')
    assert ('a', 'c') in qbf.attack_relations

    with pytest.raises(ValueError):
        qbf.add_attack_relation('d', 'c')
    with pytest.raises(ValueError):
        qbf.add_attack_relation('c', 'd')
    qbf.remove_attack_relation('d', 'c')
    qbf.remove_attack_relation('c', 'd')
    
    with pytest.raises(PermissionError):
        qbf.attack_relations.add('b', 'c')
    with pytest.raises(PermissionError):
        qbf.attack_relations.remove('a', 'c')


# TEST SUPPORT RELATIONS

def test_access_support_relations():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    assert qbf.support_relations.relations == {('a', 'b')}
    assert qbf.contains_support_relation('a', 'b')
    assert not qbf.contains_support_relation('b', 'a')
    assert not qbf.contains_support_relation('a', 'c')
    for a, b in [('a', 'c'), ('a', 'b'), ('c', 'a'), ('b', 'a')]:
        assert qbf.contains_support_relation(a,b) == qbf.support_relations.contains(a, b)
        assert qbf.contains_support_relation(a,b) == ((a,b) in qbf.support_relations)

def test_modify_support_relations():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbf.remove_support_relation('a', 'b')
    assert ('a', 'b') not in qbf.support_relations
    qbf.add_support_relation('a', 'b')
    assert ('a', 'b') in qbf.support_relations

    with pytest.raises(ValueError):
        qbf.add_support_relation('d', 'c')
    with pytest.raises(ValueError):
        qbf.add_support_relation('c', 'd')
    qbf.remove_support_relation('d', 'c')
    qbf.remove_support_relation('c', 'd')
    
    with pytest.raises(PermissionError):
        qbf.support_relations.add('b', 'c')
    with pytest.raises(PermissionError):
        qbf.support_relations.remove('a', 'b')

# TEST FINAL WEIGHTS

def test_access_final_weight():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    for argument, weight in qbf.final_weights.items():
        assert qbf.final_weight(argument) == weight
    with pytest.raises(ValueError):
        qbf.final_weight('d')
    assert qbf.final_weights == {'a': 1.0, 'c': 4.0, 'b': 2.0}

    qbfe = qbf.copy()
    assert qbfe.final_weights == {'a': 1.0, 'c': 4.0, 'b': 2.0}
    qbfe.add_argument('e', initial_weight=3)
    qbfe.add_attack_relation('e', 'c')
    assert qbfe.final_weights == {'a': 1.0, 'e': 3.0, 'c': 1.0, 'b': 2.0}

    qbfa = qbfe.copy()
    assert qbfa.final_weights == {'a': 1.0, 'e': 3.0, 'c': 1.0, 'b': 2.0}
    qbfa.remove_attack_relation('e', 'c')
    qbfa.remove_argument('e')
    qbfa.modify_initial_weight('a', 2)
    assert qbfa.final_weights == {'a': 2.0, 'c': 3.0, 'b': 3.0}

    qbf_ = qbfa.copy()
    qbf_.final_weights == {'a': 2.0, 'c': 3.0, 'b': 3.0}
    qbf_.add_argument('e', 3)
    qbf_.add_attack_relation('e', 'c')
    qbf_.add_argument('d', 1)
    qbf_.add_attack_relation('d', 'a')
    qbf_.add_support_relation('d', 'e')
    qbf_.final_weights == {'d': 1.0, 'a': 1.0, 'e': 4.0, 'c': 0.0, 'b': 2.0}

def test_modify_final_weight():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    final_weights = qbf.final_weights
    final_weights['a'] = 0.0
    assert qbf.initial_weight('a') == 1.0

    qbf.modify_initial_weight('c', 4)
    assert qbf.final_weight('c') == 3.0

    qbf.add_argument('a', 0.0)
    assert qbf.final_weight('a') == 1.0

# TEST COPY

def test_copy():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=False)
    copy = qbf.copy()
    assert qbf.arguments == copy.arguments
    assert qbf.initial_weights == copy.initial_weights
    assert qbf.attack_relations.relations == copy.attack_relations.relations
    assert qbf.support_relations.relations == copy.support_relations.relations
    assert qbf.disjoint_relations == copy.disjoint_relations
    copy.add_argument('d', 3)
    copy.modify_initial_weight('c', 3)
    copy.remove_attack_relation('a', 'c')
    copy.add_support_relation('a', 'c')
    copy.disjoint_relations = True
    assert qbf.arguments != copy.arguments
    assert qbf.initial_weights != copy.initial_weights
    assert qbf.attack_relations.relations != copy.attack_relations.relations
    assert qbf.support_relations.relations != copy.support_relations.relations
    assert qbf.disjoint_relations != copy.disjoint_relations

# TEST ACYCLIC

def test_isacyclic():
    assert QBAFramework([],[],[],[]).isacyclic()
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=False)
    assert qbf.isacyclic()
    qbf.add_attack_relation('c', 'a')
    assert not qbf.isacyclic()
    with pytest.raises(NotImplementedError):
        qbf.final_weights

# TEST EQUALS

def test_equals():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    copy = qbf.copy()
    assert qbf == copy
    assert not qbf != copy

    copy.add_argument('e',3)
    assert qbf != copy
    assert not qbf == copy

    copy = qbf.copy()
    copy.modify_initial_weight('c', 3)
    assert qbf != copy
    assert not qbf == copy

    copy = qbf.copy()
    copy.add_attack_relation('c', 'a')
    assert qbf != copy
    assert not qbf == copy

    copy = qbf.copy()
    copy.add_support_relation('b', 'a')
    assert qbf != copy
    assert not qbf == copy

def test_incorrect_compare():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    copy = qbf.copy()
    with pytest.raises(TypeError):
        qbf == 1
    with pytest.raises(TypeError):
        1 == qbf
    with pytest.raises(NotImplementedError):
        qbf < copy
    with pytest.raises(NotImplementedError):
        qbf <= copy
    with pytest.raises(NotImplementedError):
        qbf > copy
    with pytest.raises(NotImplementedError):
        qbf >= copy

# TEST STRENGTH CONSISTENCY

def test_are_strength_consistent():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbf.are_strength_consistent(qbf, 'b', 'c')
    assert qbfe.are_strength_consistent(qbfe, 'b', 'c')
    assert qbf.are_strength_consistent(qbfe, 'b', 'b')
    assert qbfe.are_strength_consistent(qbf, 'b', 'b')

    assert qbf.are_strength_consistent(qbfe, 'a', 'b')
    assert qbfe.are_strength_consistent(qbf, 'a', 'b')
    assert qbf.are_strength_consistent(qbfe, 'b', 'a')
    assert qbfe.are_strength_consistent(qbf, 'b', 'a')

    assert not qbf.are_strength_consistent(qbfe, 'b', 'c')
    assert not qbfe.are_strength_consistent(qbf, 'b', 'c')
    assert not qbf.are_strength_consistent(qbfe, 'c', 'b')
    assert not qbfe.are_strength_consistent(qbf, 'c', 'b')

def test_are_strength_consistent_error():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    with pytest.raises(TypeError):
        qbf.are_strength_consistent([], 'b', 'c')

    with pytest.raises(ValueError):
        qbf.are_strength_consistent(qbfe, 'e', 'c')
    with pytest.raises(ValueError):
        qbf.are_strength_consistent(qbfe, 'b', 'e')
    with pytest.raises(ValueError):
        qbfe.are_strength_consistent(qbf, 'e', 'c')
    with pytest.raises(ValueError):
        qbfe.are_strength_consistent(qbf, 'b', 'e')

# TEST REVERSAL

def test_reversal_input():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    qbfe.reversal(qbf, [])
    qbfe.reversal(qbf, set())

    with pytest.raises(TypeError):
        qbfe.reversal(qbf, 1)
    with pytest.raises(TypeError):
        qbfe.reversal(1, [])
    


def test_reversal_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])
    assert qbfa != qbfe
    
    assert qbfe == qbfe.reversal(qbfa, [])
    assert qbfa == qbfa.reversal(qbfe, [])

    assert qbfa == qbfe.reversal(qbfa, qbfe.arguments.union(qbfa.arguments))
    assert qbfe == qbfa.reversal(qbfe, qbfe.arguments.union(qbfa.arguments))

    assert qbfe == qbfe.reversal(qbfa, ['b', 'c'])
    assert qbfa == qbfa.reversal(qbfe, ['b', 'c'])

    assert not qbfa == qbfe.reversal(qbfa, ['e'])
    assert not qbfe == qbfa.reversal(qbfe, ['e'])

    assert not qbfa == qbfe.reversal(qbfa, ['a'])
    assert not qbfe == qbfa.reversal(qbfe, ['a'])

    assert qbfa == qbfe.reversal(qbfa, ['a', 'e'])
    assert qbfe == qbfa.reversal(qbfe, ['a', 'e'])

# TEST IS SSI EXPLANATION

def test_isSSIExplanation_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    qbfa.isSSIExplanation(qbfe, [], 'a', 'b')
    qbfa.isSSIExplanation(qbfe, set(), 'a', 'b')

    with pytest.raises(TypeError):
        qbfa.isSSIExplanation(1, [], 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isSSIExplanation(qbfe, 1, 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isSSIExplanation(qbfe, [], [], 'b')
    with pytest.raises(ValueError):
        qbfa.isSSIExplanation(qbfe, [], 'a', 'e')
    with pytest.raises(ValueError):
        qbfa.isSSIExplanation(qbfe, [], 'e', 'b')

def test_isSSIExplanation_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbfa.isSSIExplanation(qbfa, [], 'b', 'c')
    assert not qbfa.isSSIExplanation(qbfa, ['a'], 'b', 'c')

    assert qbfa.isSSIExplanation(qbfe, [], 'b', 'b')
    assert not qbfa.isSSIExplanation(qbfe, ['a'], 'b', 'b')

    for ns in [set(),{'b'},{'c'},{'b','c'}]:
        assert not qbfe.isSSIExplanation(qbfa, ns, 'b', 'c')

    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        assert not qbfa.isSSIExplanation(qbfe, ns, 'b', 'c')
    
    for ns in [set(),{'b'},{'c'},{'b','c'}]:
        for s in [{'a'},{'e'},{'a','e'}]:
            assert qbfe.isSSIExplanation(qbfa, ns.union(s), 'b', 'c')
    
    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        for s in [{'e'}]:
            assert qbfa.isSSIExplanation(qbfe, ns.union(s), 'b', 'c')

# TEST IS CSI EXPLANATION

def test_isCSIExplanation_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    qbfa.isCSIExplanation(qbfe, [], 'a', 'b')
    qbfa.isCSIExplanation(qbfe, set(), 'a', 'b')

    with pytest.raises(TypeError):
        qbfa.isCSIExplanation(1, [], 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isCSIExplanation(qbfe, 1, 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isCSIExplanation(qbfe, [], [], 'b')
    with pytest.raises(ValueError):
        qbfa.isCSIExplanation(qbfe, [], 'a', 'e')
    with pytest.raises(ValueError):
        qbfa.isCSIExplanation(qbfe, [], 'e', 'b')

def test_isCSIExplanation_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbfa.isCSIExplanation(qbfa, [], 'b', 'c')
    assert not qbfa.isCSIExplanation(qbfa, ['a'], 'b', 'c')

    assert qbfa.isCSIExplanation(qbfe, [], 'b', 'b')
    assert not qbfa.isCSIExplanation(qbfe, ['a'], 'b', 'b')

    for ns in [set(),{'a'},{'b'},{'c'},{'e'},{'a','b'},{'a','c'},{'b','c'},{'b','e'},{'c','e'},
                {'a','b','c'}, {'b','c','e'}]:
        assert not qbfe.isCSIExplanation(qbfa, ns, 'b', 'c')

    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        assert not qbfa.isCSIExplanation(qbfe, ns, 'b', 'c')
    
    for ns in [set(),{'a'},{'b'},{'c'},{'e'},{'a','b'},{'a','c'},{'b','c'},{'b','e'},{'c','e'},
                {'a','b','c'}, {'b','c','e'}]:
        assert qbfe.isCSIExplanation(qbfa, ns.union({'a','e'}), 'b', 'c')

    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        assert qbfa.isCSIExplanation(qbfe, ns.union({'e'}), 'b', 'c')

# TEST MINIMAL SSI EXPLANATIONS

def test_minimalSSIExplanations_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])
    
    qbfe.minimalSSIExplanations(qbfa, 'b', 'c')

    with pytest.raises(TypeError):
        qbfe.minimalSSIExplanations(1, 'b', 'c')
    with pytest.raises(TypeError):
        qbfe.minimalSSIExplanations(qbfe, [], 'c')

    with pytest.raises(ValueError):
        qbfe.minimalSSIExplanations(qbfe, 'x', 'c')
    with pytest.raises(ValueError):
        qbfe.minimalSSIExplanations(qbfe, 'b', 'x')

def test_minimalSSIExplanations_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbfe.minimalSSIExplanations(qbfe, 'b', 'c') == [set()]
    assert qbfe.minimalSSIExplanations(qbfa, 'b', 'b') == [set()]

    assert qbfe.minimalSSIExplanations(qbfa, 'b', 'c') in ([{'e'}, {'a'}], [{'a'}, {'e'}])
    assert qbfa.minimalSSIExplanations(qbfe, 'b', 'c') == [{'e'}]

# TEST MINIMAL CSI EXPLANATIONS

def test_minimalCSIExplanations_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])
    
    qbfe.minimalCSIExplanations(qbfa, 'b', 'c')

    with pytest.raises(TypeError):
        qbfe.minimalCSIExplanations(1, 'b', 'c')
    with pytest.raises(TypeError):
        qbfe.minimalCSIExplanations(qbfe, [], 'c')

    with pytest.raises(ValueError):
        qbfe.minimalCSIExplanations(qbfe, 'x', 'c')
    with pytest.raises(ValueError):
        qbfe.minimalCSIExplanations(qbfe, 'b', 'x')

def test_minimalCSIExplanations_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbfe.minimalCSIExplanations(qbfe, 'b', 'c') == [set()]
    assert qbfe.minimalCSIExplanations(qbfa, 'b', 'b') == [set()]

    assert qbfe.minimalCSIExplanations(qbfa, 'b', 'c') == [{'a', 'e'}]
    assert qbfa.minimalCSIExplanations(qbfe, 'b', 'c') == [{'e'}]