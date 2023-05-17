from qbaf_ctrbs.utils import determine_powerset

def test_restrict():
    args = {'a', 'b', 'c'}
    powerset = determine_powerset(args)
    lpowerset = list(powerset)
    assert len(lpowerset) == 8
    assert set() in lpowerset
    assert {'a'} in lpowerset
    assert {'b'} in lpowerset
    assert {'c'} in lpowerset
    assert {'a', 'b'} in lpowerset
    assert {'a', 'c'} in lpowerset
    assert {'b', 'c'} in lpowerset
    assert {'a', 'b', 'c'} in lpowerset