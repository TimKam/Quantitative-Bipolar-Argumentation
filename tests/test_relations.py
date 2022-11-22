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
    assert relations.relations == {(b,a), (c,a), (a,b)}
    relations.add(a,c)
    assert relations.relations == {(b,a), (c,a), (a,b), (a,c)}
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

def test_copy_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    assert copy.relations == {(a,b), (a,c)}


def test_copy_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    assert set(copy.patients(a)) == {b, c}
    assert set(copy.patients(b)) == set()
    assert set(copy.patients(c)) == set()

def test_copy_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    assert set(copy.agents(a)) == set()
    assert set(copy.agents(b)) == {a}
    assert set(copy.agents(c)) == {a}

def test_copy_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    assert copy.contains(a, b)
    assert copy.contains(a, c)
    assert not copy.contains(b, a)
    assert not copy.contains(c, a)

def test_copy_remove_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    copy = relations.copy()
    relations.remove(b,a)
    assert copy.relations == {(a,b), (a,c), (b,a), (c,a)}
    assert relations.relations == {(a,b), (a,c), (c,a)}
    copy.remove(c,a)
    assert copy.relations == {(a,b), (a,c), (b,a)}
    assert relations.relations == {(a,b), (a,c), (c,a)}
    relations.remove(c,a)
    assert copy.relations == {(a,b), (a,c), (b,a)}
    assert relations.relations == {(a,b), (a,c)}

def test_copy_remove_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    copy = relations.copy()
    relations.remove(b,a)
    copy.remove(c,a)
    assert set(relations.patients(a)) == {b, c}
    assert set(relations.patients(b)) == set()
    assert set(relations.patients(c)) == {a}
    assert set(copy.patients(a)) == {b, c}
    assert set(copy.patients(b)) == {a}
    assert set(copy.patients(c)) == set()

def test_copy_remove_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    copy = relations.copy()
    relations.remove(b,a)
    copy.remove(c,a)
    assert set(relations.agents(a)) == {c}
    assert set(relations.agents(b)) == {a}
    assert set(relations.agents(c)) == {a}
    assert set(copy.agents(a)) == {b}
    assert set(copy.agents(b)) == {a}
    assert set(copy.agents(c)) == {a}

def test_copy_remove_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c), (b,a), (c,a)])
    copy = relations.copy()
    relations.remove(b,a)
    relations.remove(c,a)
    assert relations.contains(a, b) and relations.contains(a, c)
    assert not relations.contains(b, a) and not relations.contains(c, a)
    assert copy.contains(a, b) and copy.contains(a, c)
    assert copy.contains(b, a) and copy.contains(c, a)

def test_copy_add_relations():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(b,a), (c,a)])
    copy = relations.copy()
    relations.add(a,b)
    assert relations.relations == {(b,a), (c,a), (a,b)}
    assert copy.relations == {(b,a), (c,a)}
    copy.add(a,c)
    assert relations.relations == {(b,a), (c,a), (a,b)}
    assert copy.relations == {(b,a), (c,a), (a,c)}

def test_copy_add_patients():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    relations.add(b,a)
    copy.add(c,a)
    assert set(relations.patients(a)) == {b, c}
    assert set(relations.patients(b)) == {a}
    assert set(relations.patients(c)) == set()
    assert set(copy.patients(a)) == {b, c}
    assert set(copy.patients(b)) == set()
    assert set(copy.patients(c)) == {a}

def test_copy_add_agents():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    relations.add(b,a)
    copy.add(c,a)
    assert set(relations.agents(a)) == {b}
    assert set(relations.agents(b)) == {a}
    assert set(relations.agents(c)) == {a}
    assert set(copy.agents(a)) == {c}
    assert set(copy.agents(b)) == {a}
    assert set(copy.agents(c)) == {a}

def test_copy_add_contains():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    relations.add(b,a)
    copy.add(c,a)
    assert relations.contains(a, b) and relations.contains(a, c)
    assert relations.contains(b, a) and not relations.contains(c, a)
    assert copy.contains(a, b) and copy.contains(a, c)
    assert not copy.contains(b, a) and copy.contains(c, a)

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

# TEST __EQ__

def test___eq__():
    a, b, c = Arg('a'), Arg('b'), Arg('c')
    relations = QBAFARelations([(a,b), (a,c)])
    copy = relations.copy()
    assert relations == copy
    copy.remove(a,b)
    assert relations != copy