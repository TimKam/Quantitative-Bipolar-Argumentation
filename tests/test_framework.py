import pytest
from qbaf import QBAFramework, QBAFARelations

# TEST INIT

def test_init_correct_args():
    args = ['a', 'b', 'c']
    initial_strengths = [1, 1, 5]
    att = [('a', 'c')]
    supp = [('a', 'b')]
    QBAFramework(args, initial_strengths, att, supp,
        disjoint_relations=True,
        semantics="basic_model",
        aggregation_function=None,
        influence_function=None,
        min_strength=-1.7976931348623157e+308,
        max_strength=1.7976931348623157e+308)

def test_init_semantics():
    args,initial_strengths,att,supp = ['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')]
    QBAFramework(args, initial_strengths, att, supp, semantics="basic_model")
    QBAFramework(args, initial_strengths, att, supp, semantics="QuadraticEnergy_model")
    QBAFramework(args, initial_strengths, att, supp, semantics="SquaredDFQuAD_model")
    QBAFramework(args, initial_strengths, att, supp, semantics="EulerBasedTop_model")
    QBAFramework(args, initial_strengths, att, supp, semantics="EulerBased_model")
    QBAFramework(args,[0.1, 0.1, 0.5], att, supp, semantics="DFQuAD_model")
    with pytest.raises(ValueError):
        QBAFramework(args, initial_strengths, att, supp, semantics="incorrect_model")

def test_init_no_args():
    with pytest.raises(TypeError):
        QBAFramework()

def test_init_args_initial_strengths():
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
    assert qbf.initial_strengths == {'a':1.0, 'b':1.0, 'c':5.0}
    assert qbf.attack_relations.relations == {('a', 'c')}
    assert qbf.support_relations.relations == {('a', 'b')}
    assert qbf.disjoint_relations == True
    assert qbf.final_strengths == {'a': 1.0, 'c': 4.0, 'b': 2.0}
    assert qbf.semantics == "basic_model"
    assert qbf.min_strength == -1.7976931348623157e+308
    assert qbf.max_strength ==  1.7976931348623157e+308

def test_init_setters():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=True)
    with pytest.raises(AttributeError):
        qbf.arguments = set()
    with pytest.raises(AttributeError):
        qbf.initial_strengths = dict()
    with pytest.raises(AttributeError):
        qbf.attack_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.support_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.final_strengths = dict()
    with pytest.raises(TypeError):
        qbf.disjoint_relations = 0
    qbf.disjoint_relations = False
    assert qbf.disjoint_relations == False
    qbf.add_support_relation('a', 'c')
    with pytest.raises(ValueError):
        qbf.disjoint_relations = True

# Test MAX MIN STRENGTHS

def test_init_maxmin_strengths_DFQuAD():
    args,initial_strengths,att,supp = ['a', 'b', 'c'], [0.1, 0.1, 0.5], [('a', 'c')], [('a', 'b')]
    qbf = QBAFramework(args,initial_strengths, att, supp, semantics="DFQuAD_model")
    qbf.add_argument('a', 5) # If the arguments exists it does nothing
    qbf.add_argument('a', -5) # If the arguments exists it does nothing
    with pytest.raises(ValueError):
        qbf.add_argument('e', 5)
    with pytest.raises(ValueError):
        qbf.add_argument('e', -5)
    with pytest.raises(ValueError):
        qbf.modify_initial_strength('a', 5)
    with pytest.raises(ValueError):
        qbf.modify_initial_strength('a', -5)

# TEST SEMANTICS

def test_custom_semantics_input():
    args,initial_strengths,att,supp = ['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')]
    QBAFramework(args,initial_strengths, att, supp, semantics="basic_model")
    QBAFramework(args,initial_strengths, att, supp, semantics=None)

    QBAFramework(args,initial_strengths, att, supp,
                    semantics=None,
                    aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                    influence_function=lambda w, s : w + s,
                    min_strength=-10,
                    max_strength=10)

    QBAFramework(args,initial_strengths, att, supp,
                    aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                    influence_function=lambda w, s : w + s,
                    min_strength=-10,
                    max_strength=10)

    QBAFramework(args,initial_strengths, att, supp,
                    aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                    influence_function=lambda w, s : w + s)
    
    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics="basic_model",
                    aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                    influence_function=lambda w, s : w + s,
                    min_strength=-10,
                    max_strength=10)

    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics=None,
                    aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                    influence_function=None,
                    min_strength=-10,
                    max_strength=10)
    
    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics=None,
                    aggregation_function=None,
                    influence_function=lambda w, s : w + s,
                    min_strength=-10,
                    max_strength=10)

    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics=None,
                    min_strength=-10)

    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics=None,
                    max_strength=10)

    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics="basic_model",
                    min_strength=-10)

    with pytest.raises(ValueError):
        QBAFramework(args,initial_strengths, att, supp,
                    semantics="basic_model",
                    max_strength=10)

def test_custom_semantics():
    args,initial_strengths,att,supp = ['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')]
    qbf = QBAFramework(args,initial_strengths, att, supp, semantics="basic_model")
    custom = QBAFramework(args,initial_strengths, att, supp,
                                semantics=None,
                                aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                                influence_function=lambda w, s : w + s,
                                min_strength=-10,
                                max_strength=10)
    assert custom.semantics == None
    assert qbf.final_strengths == custom.final_strengths

    custom = QBAFramework(args,initial_strengths, att, supp,
                                semantics=None,
                                aggregation_function=lambda att_s, supp_s : sum(supp_s) - sum(att_s),
                                influence_function=lambda w, s : w - s,
                                min_strength=-10,
                                max_strength=10)
    assert qbf.final_strengths != custom.final_strengths

def test_default_semantics():
    args,initial_strengths,att,supp = ['a', 'b', 'c', 'd'], [0.1, 0.1, 0.5, 0.3], [('a', 'c'), ('d', 'c')], [('a', 'b')]
    qbf = QBAFramework(args,initial_strengths, att, supp, semantics="basic_model")
    qbf2 = QBAFramework(args,initial_strengths, att, supp, semantics="QuadraticEnergy_model")
    qbf3 = QBAFramework(args,initial_strengths, att, supp, semantics="SquaredDFQuAD_model")
    qbf4 = QBAFramework(args,initial_strengths, att, supp, semantics="EulerBasedTop_model")
    qbf5 = QBAFramework(args,initial_strengths, att, supp, semantics="EulerBased_model")
    qbf6 = QBAFramework(args,initial_strengths, att, supp, semantics="DFQuAD_model")
    assert qbf.final_strengths != qbf2.final_strengths
    assert qbf3.final_strengths != qbf6.final_strengths
    assert qbf4.final_strengths != qbf5.final_strengths

# TEST ARGUMENTS

def test_modify_arguments():
    qbf = QBAFramework(['a', 'b', 'c', 'd'], [1, 1, 5, 3], [('a', 'c')], [('a', 'b')])
    assert qbf.arguments == {'a', 'b', 'c', 'd'}
    qbf.remove_argument('d')
    assert qbf.arguments == {'a', 'b', 'c'}
    qbf.add_argument('d', 1)
    assert qbf.arguments == {'a', 'b', 'c', 'd'}
    assert qbf.initial_strengths['d'] == 1.0
    with pytest.raises(ValueError):
        qbf.remove_argument('a')
    with pytest.raises(ValueError):
        qbf.remove_argument('b')
    with pytest.raises(ValueError):
        qbf.remove_argument('c')
    qbf.remove_argument('d')
    assert qbf.contains_argument('a') and qbf.contains_argument('b') and qbf.contains_argument('c')
    assert not qbf.contains_argument('d')

# TEST INITIAL STRENGTHS

def test_access_initial_strength():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    for argument, strength in qbf.initial_strengths.items():
        assert qbf.initial_strength(argument) == strength
    with pytest.raises(ValueError):
        qbf.initial_strength('d')

def test_modify_initial_strength():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    initial_strengths = qbf.initial_strengths
    initial_strengths['a'] = 0.0
    assert qbf.initial_strength('a') == 1.0

    qbf.modify_initial_strength('c', 4)
    assert qbf.initial_strength('c') == 4.0

    qbf.add_argument('a', 0.0)
    assert qbf.initial_strength('a') == 1.0

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

# TEST FINAL STRENGTHS

def test_access_final_strength():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    for argument, strength in qbf.final_strengths.items():
        assert qbf.final_strength(argument) == strength
    with pytest.raises(ValueError):
        qbf.final_strength('d')
    assert qbf.final_strengths == {'a': 1.0, 'c': 4.0, 'b': 2.0}

    qbfe = qbf.copy()
    assert qbfe.final_strengths == {'a': 1.0, 'c': 4.0, 'b': 2.0}
    qbfe.add_argument('e', initial_strength=3)
    qbfe.add_attack_relation('e', 'c')
    assert qbfe.final_strengths == {'a': 1.0, 'e': 3.0, 'c': 1.0, 'b': 2.0}

    qbfa = qbfe.copy()
    assert qbfa.final_strengths == {'a': 1.0, 'e': 3.0, 'c': 1.0, 'b': 2.0}
    qbfa.remove_attack_relation('e', 'c')
    qbfa.remove_argument('e')
    qbfa.modify_initial_strength('a', 2)
    assert qbfa.final_strengths == {'a': 2.0, 'c': 3.0, 'b': 3.0}

    qbf_ = qbfa.copy()
    qbf_.final_strengths == {'a': 2.0, 'c': 3.0, 'b': 3.0}
    qbf_.add_argument('e', 3)
    qbf_.add_attack_relation('e', 'c')
    qbf_.add_argument('d', 1)
    qbf_.add_attack_relation('d', 'a')
    qbf_.add_support_relation('d', 'e')
    qbf_.final_strengths == {'d': 1.0, 'a': 1.0, 'e': 4.0, 'c': 0.0, 'b': 2.0}

def test_modify_final_strength():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    final_strengths = qbf.final_strengths
    final_strengths['a'] = 0.0
    assert qbf.initial_strength('a') == 1.0

    qbf.modify_initial_strength('c', 4)
    assert qbf.final_strength('c') == 3.0

    qbf.add_argument('a', 0.0)
    assert qbf.final_strength('a') == 1.0

# TEST ATTACKED/SUPPORTED BY AND ATTACKERS/SUPPORTERS OF

def test_attackedBy_attackersOf_incorrect_input():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    
    with pytest.raises(TypeError):
        framework.attackedBy([])
    with pytest.raises(ValueError):
        framework.attackedBy('f')
    with pytest.raises(TypeError):
        framework.attackersOf([])
    with pytest.raises(ValueError):
        framework.attackersOf('f')

def test_attackedBy():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    for arg in ['a', 'b', 'c', 'e']:
        assert set(framework.attack_relations.patients(arg)) == set(framework.attackedBy(arg))

def test_attackersOf():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    for arg in ['a', 'b', 'c', 'e']:
        assert set(framework.attack_relations.agents(arg)) == set(framework.attackersOf(arg))

def test_supportedBy_supportersOf_incorrect_input():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    
    with pytest.raises(TypeError):
        framework.supportedBy([])
    with pytest.raises(ValueError):
        framework.supportedBy('f')
    with pytest.raises(TypeError):
        framework.supportersOf([])
    with pytest.raises(ValueError):
        framework.supportersOf('f')

def test_supportedBy():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    for arg in ['a', 'b', 'c', 'e']:
        assert set(framework.support_relations.patients(arg)) == set(framework.supportedBy(arg))

def test_supportersOf():
    framework = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3],
                            [('a', 'c'), ('e', 'c')],
                            [('a', 'b'), ('e', 'a')])
    for arg in ['a', 'b', 'c', 'e']:
        assert set(framework.support_relations.agents(arg)) == set(framework.supportersOf(arg))

# TEST COPY

def test_copy():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=False)
    copy = qbf.copy()
    assert qbf.arguments == copy.arguments
    assert qbf.initial_strengths == copy.initial_strengths
    assert qbf.attack_relations.relations == copy.attack_relations.relations
    assert qbf.support_relations.relations == copy.support_relations.relations
    assert qbf.disjoint_relations == copy.disjoint_relations
    copy.add_argument('d', 3)
    copy.modify_initial_strength('c', 3)
    copy.remove_attack_relation('a', 'c')
    copy.add_support_relation('a', 'c')
    copy.disjoint_relations = True
    assert qbf.arguments != copy.arguments
    assert qbf.initial_strengths != copy.initial_strengths
    assert qbf.attack_relations.relations != copy.attack_relations.relations
    assert qbf.support_relations.relations != copy.support_relations.relations
    assert qbf.disjoint_relations != copy.disjoint_relations

def test_copy_semantics():
    args,initial_strengths,att,supp = ['a', 'b', 'c'], [0.1, 0.1, 0.5], [('a', 'c')], [('a', 'b')]
    qbf = QBAFramework(args,initial_strengths, att, supp, semantics="DFQuAD_model")
    copy = qbf.copy()
    assert qbf.semantics == copy.semantics
    assert qbf.min_strength == copy.min_strength
    assert qbf.max_strength == copy.max_strength

# TEST ACYCLIC

def test_isacyclic():
    assert QBAFramework([],[],[],[]).isacyclic()
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')], disjoint_relations=False)
    assert qbf.isacyclic()
    qbf.add_attack_relation('c', 'a')
    assert not qbf.isacyclic()
    with pytest.raises(NotImplementedError):
        qbf.final_strengths

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
    copy.modify_initial_strength('c', 3)
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

# TEST IS NSI EXPLANATION

def test_isNSIExplanation_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    qbfa.isNSIExplanation(qbfe, [], 'a', 'b')
    qbfa.isNSIExplanation(qbfe, set(), 'a', 'b')

    with pytest.raises(TypeError):
        qbfa.isNSIExplanation(1, [], 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isNSIExplanation(qbfe, 1, 'a', 'b')
    with pytest.raises(TypeError):
        qbfa.isNSIExplanation(qbfe, [], [], 'b')
    with pytest.raises(ValueError):
        qbfa.isNSIExplanation(qbfe, [], 'a', 'e')
    with pytest.raises(ValueError):
        qbfa.isNSIExplanation(qbfe, [], 'e', 'b')

def test_isNSIExplanation_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])

    assert qbfa.isNSIExplanation(qbfa, [], 'b', 'c')
    assert not qbfa.isNSIExplanation(qbfa, ['a'], 'b', 'c')

    assert qbfa.isNSIExplanation(qbfe, [], 'b', 'b')
    assert not qbfa.isNSIExplanation(qbfe, ['a'], 'b', 'b')

    for ns in [set(),{'a'},{'b'},{'c'},{'e'},{'a','b'},{'a','c'},{'b','c'},{'b','e'},{'c','e'},
                {'a','b','c'}, {'b','c','e'}]:
        assert not qbfe.isNSIExplanation(qbfa, ns, 'b', 'c')

    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        assert not qbfa.isNSIExplanation(qbfe, ns, 'b', 'c')
    
    for ns in [set(),{'a'},{'b'},{'c'},{'e'},{'a','b'},{'a','c'},{'b','c'},{'b','e'},{'c','e'},
                {'a','b','c'}, {'b','c','e'}]:
        assert qbfe.isNSIExplanation(qbfa, ns.union({'a','e'}), 'b', 'c')

    for ns in [set(),{'a'},{'b'},{'c'},{'a','b'},{'a','c'},{'b','c'},{'a','b','c'}]:
        assert qbfa.isNSIExplanation(qbfe, ns.union({'e'}), 'b', 'c')

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

# TEST MINIMAL NSI EXPLANATIONS

def test_minimalNSIExplanations_input():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])
    
    qbfe.minimalNSIExplanations(qbfa, 'b', 'c')

    with pytest.raises(TypeError):
        qbfe.minimalNSIExplanations(1, 'b', 'c')
    with pytest.raises(TypeError):
        qbfe.minimalNSIExplanations(qbfe, [], 'c')

    with pytest.raises(ValueError):
        qbfe.minimalNSIExplanations(qbfe, 'x', 'c')
    with pytest.raises(ValueError):
        qbfe.minimalNSIExplanations(qbfe, 'b', 'x')

def test_minimalNSIExplanations_output():
    qbfa = QBAFramework(['a', 'b', 'c'], [2, 1, 5], [('a', 'c')], [('a', 'b')])
    qbfe = QBAFramework(['a', 'b', 'c', 'e'], [1, 1, 5, 3], [('a', 'c'), ('e', 'c')], [('a', 'b')])
    qbf_ = QBAFramework(['a', 'b', 'c', 'e', 'd'], [2, 1, 5, 3, 1], [('a', 'c'), ('e', 'c'), ('d', 'a')], [('a', 'b'), ('d', 'e')])

    assert qbfe.minimalNSIExplanations(qbfe, 'b', 'c') == [set()]
    assert qbfe.minimalNSIExplanations(qbfa, 'b', 'b') == [set()]

    assert qbfe.minimalNSIExplanations(qbfa, 'b', 'c') == [{'a', 'e'}]
    assert qbfa.minimalNSIExplanations(qbfe, 'b', 'c') == [{'e'}]

    assert qbf_.minimalNSIExplanations(qbfa, 'b', 'c') == [{'d', 'e'}]
    assert qbfa.minimalNSIExplanations(qbf_, 'b', 'c') == [{'e'}]