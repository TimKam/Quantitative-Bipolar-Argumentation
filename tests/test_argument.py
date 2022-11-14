import pytest
from qbaf import QBAFArgument

def test_init_correct_args():
    QBAFArgument('a')
    QBAFArgument(name='a')
    QBAFArgument('a', 'desc')
    QBAFArgument(name='a', description='desc')

def test_init_no_args():
    with pytest.raises(TypeError):
        QBAFArgument()

def test_name_init():
    argument = QBAFArgument('a', 'desc')
    assert argument.name == 'a'

def test_description_init():
    argument = QBAFArgument('a', 'desc')
    assert argument.description == 'desc'

def test_description_default_init():
    argument = QBAFArgument('a')
    assert argument.description == ''

def test_name_set():
    with pytest.raises(AttributeError):
        argument = QBAFArgument('a')
        argument.name = 'b'

def test_description_set():
    argument = QBAFArgument('a', 'd')
    argument.description = 'desc'
    assert argument.description == 'desc'

def test_name_type():
    with pytest.raises(TypeError):
        QBAFArgument(1, 'desc')

def test_description_type():
    with pytest.raises(TypeError):
        QBAFArgument('a', 2)

def test_equal_different_description():
    argument = QBAFArgument('a', 'desc')
    assert argument == QBAFArgument('a', 'different')

def test_not_equal_different_name():
    argument = QBAFArgument('a', 'desc')
    assert argument != QBAFArgument('b', 'desc')

def test_equal_same_as_name():
    for x in range(20):
        for y in range(20):
            if str(x) == str(y):
                assert QBAFArgument(str(x), 'desc') == QBAFArgument(str(y), 'desc')
            else:
                assert QBAFArgument(str(x), 'desc') != QBAFArgument(str(y), 'desc')

def test_hash():
    for x in range(20):
        assert hash(QBAFArgument(str(x), 'desc')) == hash(str(x))
