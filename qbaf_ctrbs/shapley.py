import math
from qbaf_ctrbs.utils import restrict, determine_powerset

def determine_shapley_ctrb(topic, contributor, qbaf):
    """Determines the shapley contribution of a contributor
    to a topic argument.

    Args:
        topic (string): The topic argument
        contributor (string): The contributing argument
        qbaf (QBAFramework): The QBAF that contains topic and contributor

    Returns:
        float: The contribution of the contributor to the topic
    """
    if topic == contributor:
        raise Exception (
            'An argument\'s shapley contribution to itself cannot be determined.')
    if not topic in qbaf.arguments or not contributor in qbaf.arguments:
        raise Exception ('Topic and contributor must be in the QBAF.')
    sub_ctrbs = []
    reduced_args = [arg for arg in qbaf.arguments if arg not in [contributor, topic]]
    subsets = determine_powerset(reduced_args)
    for subset in subsets:
        lsubset = list(subset)
        qbaf_without = restrict(qbaf, [arg for arg in qbaf.arguments if arg in lsubset + [topic]])
        qbaf_with = restrict(qbaf, [arg for arg in qbaf.arguments if arg in lsubset + [topic, contributor]])
        weight = (math.factorial(len(lsubset)) * math.factorial(len(qbaf.arguments)-2-len(lsubset)))/math.factorial(len(qbaf.arguments)-1)
        sub_ctrb = weight * (qbaf_with.final_strengths[topic] - qbaf_without.final_strengths[topic])
        sub_ctrbs.append(sub_ctrb)
    return sum(sub_ctrbs)