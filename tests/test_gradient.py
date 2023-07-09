from qbaf import QBAFramework
from qbaf_ctrbs.gradient import determine_gradient_ctrb

def test_gradient():
    args = ['a', 'b', 'c', 'd', 'e']
    initial_strengths = [0.5, 0, 0, 0, 0.5]
    atts = [('b', 'e'), ('c', 'e')]
    supps = [('a', 'b'), ('a', 'c'), ('a', 'd'), ('d', 'e')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='DFQuAD_model')
    assert round(determine_gradient_ctrb('e', 'a', qbaf), 7) == 0