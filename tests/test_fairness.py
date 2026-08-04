from qbaf import QBAFramework
from qbaf_solf.fairness_notions import calculate_gini_fairness, calculate_shannon_fairness, is_live_fair, is_ideal_fair, is_cautious_fair

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

def test_gini_fairness():
    gini_1 = calculate_gini_fairness([Q, Q_1, Q_2], ['b', 'c', 'a'], 0.51)
    gini_2 = calculate_gini_fairness([Q, Q_1], ['b', 'c', 'a'], 0.51)

    assert (gini_1 > 0.76 and gini_1 < 0.77)
    assert (gini_2 > 0.46 and gini_2 < 0.47)


def test_shannon_fairness():
    shannon_1 = calculate_shannon_fairness([Q, Q_1, Q_2], ['b', 'c', 'a'], 0.51)
    shannon_2 = calculate_shannon_fairness([Q, Q_1], ['b', 'c', 'a'], 0.51)

    assert (shannon_1 > 0.57 and shannon_1 < 0.58)
    assert (shannon_2 > 0.63 and shannon_2 < 0.64)

def test_ideal_fairness():

    assert is_ideal_fair([Q, Q_1, Q_2], ['a', 'c'], 0.15) == False
    assert is_ideal_fair([Q], ['a', 'c'], 0.1) == True


def test_live_fairness():

    assert is_live_fair([Q, Q_1, Q_2], ['a', 'b'], 0.15) == True
    assert is_live_fair([Q], ['a', 'b'], 0.1) == True


def test_cautious_fairness():

    assert is_cautious_fair([Q, Q_1, Q_2], ['a', 'c'], 0.4) == True
    assert is_cautious_fair([Q], ['a', 'c'], 0.4) == False



