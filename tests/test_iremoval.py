import pytest
from qbaf import QBAFramework
from qbaf_ctrbs.intrinsic_removal import determine_iremoval_ctrb

def test_iremoval():
    args = ['a', 'b', 'c']
    is_a = 2
    is_b = 1
    is_c = 1
    initial_strengths = [is_a, is_b, is_c]
    atts = [('b', 'a')]
    supps = [('c', 'b')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='basic_model')
    assert determine_iremoval_ctrb('a', 'b', qbaf) == -1    
    assert determine_iremoval_ctrb('a', 'c', qbaf) == -1
    assert determine_iremoval_ctrb('b', 'a', qbaf) == 0