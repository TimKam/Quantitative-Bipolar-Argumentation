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
        semantics="naive",
        aggregation_function=None,
        influence_function=None,
        min_weight=-1.7976931348623157e+308,
        max_weight=1.7976931348623157e+308)

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
    for a, b in [('a', 'c'), ('a', 'b'), ('c', 'a'), ('b', 'a')]:
        assert qbf.contains_attack_relation(a,b) == qbf.attack_relations.contains(a, b)
        assert qbf.contains_attack_relation(a,b) == ((a,b) in qbf.attack_relations)

def test_modify_attack_relations():
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    qbf.remove_attack_relation('a', 'c')
    assert ('a', 'c') not in qbf.attack_relations
    qbf.add_attack_relation('a', 'c')
    assert ('a', 'c') in qbf.attack_relations
    
    with pytest.raises(PermissionError):
        qbf.attack_relations.add('b', 'c')
    with pytest.raises(PermissionError):
        qbf.attack_relations.remove('a', 'c')


# TEST SUPPORT RELATIONS