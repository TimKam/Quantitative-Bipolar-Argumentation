import pytest

from qbaf import QBAFramework
from qbaf_ctrbs.shapley import determine_shapley_ctrb, determine_partitioned_shapley_ctrb



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


##########################
# Partitioned shapley
##########################
def make_qbaf():
    args = ['a', 'b', 'c']
    strengths = [2, 1, 1]
    atts = [('b', 'a')]
    supps = [('c', 'b')]
    return QBAFramework(args, strengths, atts, supps, semantics='basic_model')

def test_trivial_partition_matches_unpartitioned():
    qbaf = make_qbaf()
    partition = [{'a'}, {'b'}, {'c'}]
    for contributor in [{'b'}, {'c'}]:
        expected = determine_shapley_ctrb('a', contributor, qbaf)
        got = determine_partitioned_shapley_ctrb('a', contributor, partition, qbaf)
        assert got == expected

def test_grouped_partition_for_two_contributors():
    qbaf = make_qbaf()
    partition = [{'a','b'}, {'c'}]
    expected = determine_shapley_ctrb('c', {'a','b'}, qbaf)
    got = determine_partitioned_shapley_ctrb('c', {'a','b'}, partition, qbaf)
    assert got == expected

def test_error_on_invalid_partition():
    qbaf = make_qbaf()
    bad_partition = [{'a'}, {'b'}] # Missing {'c'}.
    with pytest.raises(Exception): determine_partitioned_shapley_ctrb('a', {'b'}, bad_partition, qbaf)

    bad_partition = [{'a'}, {'c'}] # Missing {'b'}.
    with pytest.raises(Exception): determine_partitioned_shapley_ctrb('a', {'c'}, bad_partition, qbaf)

    bad_partition = [{'a'}, {'b'}, {'c'}, {'d'}] # Extra {'d'}.
    with pytest.raises(Exception): determine_partitioned_shapley_ctrb('a', {'b'}, bad_partition, qbaf)

    bad_partition = [{'a'}, {'b', 'c'}, {'c'}] # Not disjoint.
    with pytest.raises(Exception): determine_partitioned_shapley_ctrb('a', {'b', 'c'}, bad_partition, qbaf)

    # Contributor not in partition.
    bad_partition = [{'a'}, {'b'}, {'c'}]
    with pytest.raises(Exception): determine_partitioned_shapley_ctrb('a', {'b','c'}, bad_partition, qbaf)

def test_single_arg_partition():
    qbaf = make_qbaf()
    partition = [{'a'}, {'b'}, {'c'}]
    assert determine_partitioned_shapley_ctrb('a', {'b'}, partition, qbaf) == -1.5
    assert determine_partitioned_shapley_ctrb('a', {'c'}, partition, qbaf) == -0.5
    assert determine_partitioned_shapley_ctrb('b', {'a'}, partition, qbaf) == 0

def test_multi_arg_partition():
    qbaf = make_qbaf()
    partition = [{'a', 'b'}, {'c'}]
    assert determine_partitioned_shapley_ctrb('c', {'a', 'b'}, partition, qbaf) == 0

    partition = [{'a'}, {'b', 'c'}]
    assert determine_partitioned_shapley_ctrb('a', {'b', 'c'}, partition, qbaf) == -2