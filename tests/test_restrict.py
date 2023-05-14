import pytest
from qbaf import QBAFramework
from qbaf_ctrbs.utils import restrict

def test_restrict():
    args = ['a', 'b', 'c']
    is_a = 0.5
    is_b = 0.1
    is_c = 0.9
    initial_strengths = [is_a, is_b, is_c]
    atts = [('b', 'a'), ('c', 'b')]
    supps = [('c', 'a')]
    qbaf_0 = QBAFramework(args, initial_strengths,
                          atts, supps, semantics='QuadraticEnergy_model')
    qbaf_1 = restrict(qbaf_0, ['a', 'c'])
    assert qbaf_1.arguments == {'a', 'c'}
    assert qbaf_1.attack_relations.contains('c', 'b') == False
    assert qbaf_1.attack_relations.contains('b', 'a') == False
    assert qbaf_1.support_relations.contains('c', 'a') == True
    assert qbaf_1.initial_strengths == {'a': 0.5, 'c': 0.9}