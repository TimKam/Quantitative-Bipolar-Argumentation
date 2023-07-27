from qbaf import QBAFramework
from qbaf_ctrbs.gradient import determine_gradient_ctrb

def test_gradient():
    args = ['a', 'b', 'c', 'd', 'e']
    initial_strengths = [0.5, 0, 0, 0, 0.5]
    atts = [('b', 'e'), ('c', 'e')]
    supps = [('a', 'b'), ('a', 'c'), ('a', 'd'), ('d', 'e')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='DFQuAD_model')
    assert round(determine_gradient_ctrb('e', 'a', qbaf), 7) == 0

def test_initial_vs_final_gradient():
    # test bug fix: epsilon used to be applied to contributor's final strength
    args = ['a', 'b', 'c', 'd']
    initial_strengths = [0.29, 0.45, 0.45, 0.3]
    atts = [('d', 'b')]
    supps = [('b', 'a'), ('c', 'a')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics=f"EulerBasedTop_model")
    ctrb_b_to_a = determine_gradient_ctrb('a', 'b', qbaf)
    ctrb_d_to_a = determine_gradient_ctrb('a', 'd', qbaf)
    assert round(ctrb_b_to_a, 7) == round(0.2078368291258812, 7)
    assert round(ctrb_d_to_a, 7) == round(-0.028916746377944946, 7)