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

    This is a wrapper around determine_partitioned_shapley_ctrb.
    It partitions qbaf.arguments into singletons,
    except for contributors which form one set, then calls determine_partitioned_shapley_ctrb.
    """
    partition = {frozenset([a]) for a in qbaf.arguments if a not in contributors and a != topic} | {frozenset([topic]), frozenset(contributors)}
    return determine_partitioned_shapley_ctrb(topic, contributors, partition, qbaf)


def determine_partitioned_shapley_ctrb(topic, contributors, partition, qbaf):
    """Determines the shapley contribution of a contributor
    or a set of contributors to a topic argument for a given argument partition.

    Args:
        topic (string): The topic argument
        contributors (string or set): The contributing argument(s)
        partition (set): The argument partitioning
        qbaf (QBAFramework): The QBAF that contains topic and contributor

    Returns:
        float: The contribution of the contributor to the topic
    """
    if not isinstance(contributors, set):
        contributors = {contributors}
    if topic in contributors:
        raise Exception ('An argument\'s shapley contribution to itself cannot be determined.')
    if not all(item in qbaf.arguments for item in [topic, *contributors]):
            raise Exception ('Topic and contributor must be in the QBAF.')
    if ({topic} not in partition) or (contributors not in partition):
         raise Exception ('Topic and contributor must be part of the given partition.')

    partitioned_args = set().union(*partition) # Flatten list of sets into one set.
    if partitioned_args != set(qbaf.arguments):
         raise Exception('The partition is incomplete.')
    total_len = sum(len(s) for s in partition)
    if total_len != len(partitioned_args):
         raise Exception('Too many arguments in the partition (might not be disjoint).')

    sub_ctrbs = []
    reduced_partition = [frozenset(part) for part in partition if part not in [{topic}, contributors]]
    subsets = determine_powerset(reduced_partition)
    for subset in subsets:
        targets = {topic} | set().union(*subset)
        qbaf_without = restrict(qbaf, list(targets))

        targets |= contributors
        qbaf_with = restrict(qbaf, list(targets))

        weight = (math.factorial(len(subset)) * math.factorial((len(partition) - 1) - len(subset) - 1)
                  ) / math.factorial(len(partition) - 1)
        sub_ctrb = weight * (qbaf_with.final_strengths[topic] - qbaf_without.final_strengths[topic])
        sub_ctrbs.append(sub_ctrb)
    return sum(sub_ctrbs)