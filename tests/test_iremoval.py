import pytest
from qbaf import QBAFramework
from qbaf_ctrbs.intrinsic_removal import determine_iremoval_ctrb

def test_iremoval():
    args = ['a', 'b', 'c', 'd']
    is_a = 2
    is_b = 1
    is_c = 1
    is_d = 1
    initial_strengths = [is_a, is_b, is_c, is_d]
    atts = [('b', 'a'), ('d','c')]
    supps = [('c', 'b')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='basic_model')
    assert determine_iremoval_ctrb('a', 'b', qbaf) == -1    
    assert determine_iremoval_ctrb('a', 'c', qbaf) == -1
    assert determine_iremoval_ctrb('b', 'a', qbaf) == 0
    assert determine_iremoval_ctrb('a', {'b','c'}, qbaf) == -2
    assert determine_iremoval_ctrb('b', {'a','c'}, qbaf) == 1