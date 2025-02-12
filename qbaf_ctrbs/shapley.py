import math
from qbaf_ctrbs.utils import restrict, determine_powerset

def determine_shapley_ctrb(topic, contributors, qbaf):
    """Determines the shapley contribution of a contributor
    or a set of contributors to a topic argument.
    
    .. note::
        If a set of contributors is provided, its elements are treated 
        as a single player in the calculation. For example, given three 
        arguments {a}, {b}, and {c}, specifying {b, c} as a contributor 
        results in two players: {a} and {b, c}.

    Args:
        topic (string): The topic argument
        contributors (string or set): The contributing argument(s)
        qbaf (QBAFramework): The QBAF that contains topic and contributor

    Returns:
        float: The contribution of the contributor to the topic
    """
    if topic in contributors:
        raise Exception (
            'An argument\'s shapley contribution to itself cannot be determined.')
    if not all(item in qbaf.arguments for item in [topic, *contributors]):
            raise Exception ('Topic and contributor must be in the QBAF.')
    if not isinstance(contributors, set):
        contributors = {contributors}
    sub_ctrbs = []
    reduced_args = [arg for arg in qbaf.arguments if arg not in [*contributors, topic]]
    subsets = determine_powerset(reduced_args)
    for subset in subsets:
        lsubset = list(subset)
        qbaf_without = restrict(qbaf, [arg for arg in qbaf.arguments if arg in lsubset + [topic]])
        qbaf_with = restrict(qbaf, [arg for arg in qbaf.arguments if arg in lsubset + [topic, *contributors]])
        weight = (math.factorial(len(lsubset)) * math.factorial(len(qbaf.arguments)-1-len(lsubset)-len(contributors)))/math.factorial(len(qbaf.arguments)-1-len(contributors)+1)
        sub_ctrb = weight * (qbaf_with.final_strengths[topic] - qbaf_without.final_strengths[topic])
        sub_ctrbs.append(sub_ctrb)
    return sum(sub_ctrbs)