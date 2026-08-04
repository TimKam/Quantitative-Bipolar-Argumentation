from qbaf_solf.safety_oscillations_liveness import *

args = ['a', 'b', 'c']
args_1 = ['a', 'b', 'c', 'd']
args_2 = ['a', 'b', 'c', 'd', 'e']

supp = [('c', 'a')]
supp_1 = [('c', 'a')]
supp_2 = [('c', 'a')]

att = []
att_1 = [('d', 'a'), ('d', 'b')]
att_2 = [('d', 'a'), ('d', 'b'), ('e', 'd')]

Q = QBAFramework(args, [0.5, 0.7, 0.2], att, supp, semantics="DFQuAD_model")
Q_1 = QBAFramework(args_1, [0.5, 0.7, 0.2, 1.0], att_1, supp_1, semantics="DFQuAD_model")
Q_2 = QBAFramework(args_2, [0.5, 0.7, 0.2, 1.0, 0.8], att_2, supp_2, semantics="DFQuAD_model")

def test_safety():

    assert is_safe([Q, Q_1, Q_2], ['b', 'c', 'a'], 0.6) == False
    assert is_safe([Q, Q_1], ['c'], 0.1) == True


def test_liveness():

    assert is_live([Q, Q_1, Q_2], ['b', 'c', 'a'], 0.1) == True
    assert is_live([Q, Q_1], ['b', 'c', 'a'], 0.9) == False



def test_oscillations():

    assert number_of_oscillations([Q, Q_1, Q_2], ['b', 'c', 'a'], 0.5) == {'a': 2, 'b': 2, 'c':0}
    assert number_of_oscillations([Q, Q_1], ['b', 'c', 'a'], 0.9) == {'a': 0, 'b': 0, 'c': 0}

