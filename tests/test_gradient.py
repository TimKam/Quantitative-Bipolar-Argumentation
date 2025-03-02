from qbaf import QBAFramework
from qbaf_ctrbs.gradient import determine_gradient_ctrb


def test_gradient_set():
    args = ['a', 'b', 'c', 'd', 'e']
    initial_strengths = [0.5, 0, 0, 0, 0.5]
    atts = [('b', 'e'), ('c', 'e')]
    supps = [('a', 'b'), ('a', 'c'), ('a', 'd'), ('d', 'e')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='DFQuAD_model')

    assert determine_gradient_ctrb('e', {'a', 'b'}, qbaf) >= determine_gradient_ctrb('e', {'b'}, qbaf)
    assert determine_gradient_ctrb('e', {'b', 'c', 'd'}, qbaf) >= determine_gradient_ctrb('e', {'b','c'}, qbaf)

def test_gradient():
    args = ['a', 'b', 'c', 'd', 'e']
    initial_strengths = [0.5, 0, 0, 0, 0.5]
    atts = [('b', 'e'), ('c', 'e')]
    supps = [('a', 'b'), ('a', 'c'), ('a', 'd'), ('d', 'e')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='DFQuAD_model')
    assert round(determine_gradient_ctrb('e', 'a', qbaf), 7) == 0

def test_gradient_edge_case():
    args = ['a', 'b', 'c']
    initial_strengths = [0.1, 1, 1]
    atts = [('b', 'a'), ('c', 'a')]
    supps = []
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics="DFQuAD_model")
    assert determine_gradient_ctrb('a', 'b', qbaf) == 0
    assert determine_gradient_ctrb('a', 'c', qbaf) == 0

def test_gradient_top_edge_case():
    # if two contributors with maximum strength (and only them) attack or support a topic argument,
    # their contributions are expected to zero.
    args = ['a', 'b', 'c']
    initial_strengths = [0.1, 1, 1]
    atts = [('b', 'a'), ('c', 'a')]
    supps = []
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics="EulerBasedTop_model")
    assert determine_gradient_ctrb('a', 'c', qbaf) == 0
    assert determine_gradient_ctrb('a', 'b', qbaf) == 0
    