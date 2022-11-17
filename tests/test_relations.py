import pytest
from qbaf import QBAFARelations
from qbaf import QBAFArgument as Arg

# TEST INIT

def test_init_correct_args():
    a = Arg('a')
    b = Arg('b')
    QBAFARelations(set())
    QBAFARelations([])
    tuple = (a,b)
    QBAFARelations({tuple})
    QBAFARelations([tuple])

def test_init_no_args():
    with pytest.raises(TypeError):
        QBAFARelations()

def test_init_no_collection():
    with pytest.raises(TypeError):
        QBAFARelations(1)

def test_init_incorrect_tuple_size():
    with pytest.raises(TypeError):
        QBAFARelations({(1,2,3)})
    with pytest.raises(TypeError):
        QBAFARelations([(1,2,3)])

def test_init_non_hashable():
    with pytest.raises(TypeError):
        QBAFARelations({([],[])})

def test_init_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert relations.relations == {(a,b), (a,c)}

def test_init_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert set(relations.patients(a)) == {b, c}
    assert set(relations.patients(b)) == set()
    assert set(relations.patients(c)) == set()

def test_init_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert set(relations.agents(a)) == set()
    assert set(relations.agents(b)) == {a}
    assert set(relations.agents(c)) == {a}

def test_init_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert relations.contains(a, b)
    assert relations.contains(a, c)
    assert not relations.contains(b, a)
    assert not relations.contains(c, a)

def test_init___contains__():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert (a, b) in relations
    assert (a, c) in relations
    assert (b, a) not in relations
    assert (c, a) not in relations

def test___contains___incorrect_arguments():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    with pytest.raises(TypeError):
        a in relations
    with pytest.raises(TypeError):
        (a, b, c) in relations
    with pytest.raises(TypeError):
        (a, []) in relations

# TEST REMOVE

def test_remove_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    relations.remove(b,a)
    assert relations.relations == {(a,b), (a,c), (c,a)}
    relations.remove(c,a)
    assert relations.relations == {(a,b), (a,c)}

def test_remove_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    relations.remove(b,a)
    relations.remove(c,a)
    assert set(relations.patients(a)) == {b, c}
    assert set(relations.patients(b)) == set()
    assert set(relations.patients(c)) == set()

def test_remove_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    relations.remove(b,a)
    relations.remove(c,a)
    assert set(relations.agents(a)) == set()
    assert set(relations.agents(b)) == {a}
    assert set(relations.agents(c)) == {a}

def test_remove_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    relations.remove(b,a)
    relations.remove(c,a)
    assert relations.contains(a, b)
    assert relations.contains(a, c)
    assert not relations.contains(b, a)
    assert not relations.contains(c, a)

# TEST ADD & REMOVE

def test_add_remove_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(b,a), (c,a)])
    relations.add(a,b)
    relations.relations == {(b,a), (c,a), (a,b)}
    relations.add(a,c)
    relations.relations == {(b,a), (c,a), (a,b), (a,c)}
    relations.remove(b,a)
    relations.remove(c,a)
    assert relations.relations == {(a,b), (a,c)}

def test_add_remove_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(b,a), (c,a)])
    relations.add(a,b)
    relations.remove(b,a)
    relations.add(a,c)
    relations.remove(c,a)
    assert set(relations.patients(a)) == {b, c}
    assert set(relations.patients(b)) == set()
    assert set(relations.patients(c)) == set()

def test_add_remove_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(b,a), (c,a)])
    relations.add(a,b)
    relations.remove(b,a)
    relations.add(a,c)
    relations.remove(c,a)
    assert set(relations.agents(a)) == set()
    assert set(relations.agents(b)) == {a}
    assert set(relations.agents(c)) == {a}

def test_add_remove_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(b,a), (c,a)])
    relations.add(a,b)
    relations.remove(b,a)
    relations.add(a,c)
    relations.remove(c,a)
    assert relations.contains(a, b)
    assert relations.contains(a, c)
    assert not relations.contains(b, a)
    assert not relations.contains(c, a)

# TEST COPY

# TODO

# TEST LEN
def test_len():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    assert len(relations) == 2
    assert len(relations) == len(relations.relations)

# TEST ISDISJOINT

def test_isdisjoint():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    rel1 = QBAFARelations([(a,b), (a,c)])
    rel2 = QBAFARelations([(a,b), (c,a)])
    rel3 = QBAFARelations([(a,c), (b,a)])
    assert not rel1.isdisjoint(rel2)
    assert not rel1.isdisjoint(rel3)
    assert rel2.isdisjoint(rel3)

def test_isdisjoint_incorrect_arg():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    with pytest.raises(TypeError):
        relations.isdisjoint(1)