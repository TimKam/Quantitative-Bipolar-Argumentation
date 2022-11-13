import pytest
from qbaf import QBAFArgument

def test_name():
    argument = QBAFArgument('a', 'desc')
    assert argument.name == 'a'