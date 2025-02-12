from qbaf import QBAFramework
from qbaf_ctrbs.shapley import determine_shapley_ctrb

def test_shapley():
    args = ['a', 'b', 'c']
    is_a = 2
    is_b = 1
    is_c = 1
    initial_strengths = [is_a, is_b, is_c]
    atts = [('b', 'a')]
    supps = [('c', 'b')]
    qbaf = QBAFramework(args, initial_strengths, atts, supps, semantics='basic_model')
    assert determine_shapley_ctrb('a', 'b', qbaf) == -1.5
    assert determine_shapley_ctrb('a', 'c', qbaf) == -0.5
    assert determine_shapley_ctrb('b', 'a', qbaf) == 0
    assert determine_shapley_ctrb('c', {'a', 'b'}, qbaf) == 0
    assert determine_shapley_ctrb('c', {'a', 'b'}, qbaf) == 0
    assert determine_shapley_ctrb('b', {'c', 'a'}, qbaf) == 1
    
    # Disconnected null-player should have no effect.
    args.append('d') # Add null-player (no atts & no supps).
    initial_strengths.append(0) # Make initial strength of d 0.
    qbaf_bigger = QBAFramework(args, initial_strengths, atts, supps, semantics='basic_model')
    assert determine_shapley_ctrb('a', 'b', qbaf_bigger) == determine_shapley_ctrb('a', 'b', qbaf)
    assert determine_shapley_ctrb('b', {'c', 'a'}, qbaf_bigger) == determine_shapley_ctrb('b', {'c', 'a'}, qbaf)