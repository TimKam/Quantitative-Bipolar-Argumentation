from itertools import chain, combinations
from qbaf import QBAFramework

def restrict(qbaf, arguments):
    """Restricts the given QBAF to the provided set of arguments.

    Args:
        qbaf (QBAFramework): QBAF that is to be restricted
        arguments (list): Arguments the QBAF is to be restricted to (all other arguments will be removed)

    Returns:
        QBAFramework: restricted QBAF
    """
    restriction_is = []
    new_args = []
    for arg, strength in qbaf.initial_strengths.items():
        if arg in arguments:
            new_args.append(arg)
            restriction_is.append(strength)
    restriction_atts = [(source, target) for source, target in qbaf.attack_relations.relations if source in arguments and target in arguments]
    restriction_supps = [(source, target) for source, target in qbaf.support_relations.relations if source in arguments and target in arguments]
    return QBAFramework(new_args, restriction_is, restriction_atts, restriction_supps, semantics=qbaf.semantics)

def determine_powerset(elements):
    """Determines the powerset of a list of elements

    Args:
        elements (set): Set of elements

    Returns:
        list: Powerset of elements
    """
    lset = list(elements)
    ps_elements = chain.from_iterable(combinations(lset, option) for option in range(len(lset) + 1))
    return [set(ps_element) for ps_element in ps_elements]