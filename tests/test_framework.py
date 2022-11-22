import pytest
from qbaf import QBAFramework, QBAFARelations

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
    qbf = QBAFramework(['a', 'b', 'c'], [1, 1, 5], [('a', 'c')], [('a', 'b')])
    with pytest.raises(AttributeError):
        qbf.arguments = set()
    with pytest.raises(AttributeError):
        qbf.initial_weights = dict()
    with pytest.raises(AttributeError):
        qbf.attack_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.support_relations = QBAFARelations([])
    with pytest.raises(AttributeError):
        qbf.disjoin_relations = False
    with pytest.raises(AttributeError):
        qbf.final_weights = dict()

