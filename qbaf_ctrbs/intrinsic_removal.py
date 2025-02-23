from qbaf import QBAFramework
from qbaf_ctrbs.utils import restrict

def determine_iremoval_ctrb(topic, contributors, qbaf):
    """Determines the intrinsic removal contribution of a contributor
     or a set of contributors to a topic argument.

    Args:
        topic (string): The topic argument
        contributors (string or set): The contributing argument(s)
        qbaf (QBAFramework): The QBAF that contains topic and contributor

    Returns:
        float: The contribution of the contributor to the topic
    """
    if topic in contributors:
        raise Exception (
            'An argument\'s intrinsic removal contribution to itself cannot be determined.')
    if not all(item in qbaf.arguments for item in [topic, *contributors]):
        raise Exception ('Topic and contributor must be in the QBAF.')
    if not isinstance(contributors, set):
        contributors = {contributors}
    attackers = [(source, target) for source, target in qbaf.attack_relations.relations if (source in contributors or target not in contributors)]
    supporters = [(source, target) for source, target in qbaf.support_relations.relations if (source in contributors or target not in contributors)]
    arguments = list(qbaf.arguments)
    initial_strengths = [qbaf.initial_strengths[argument] for argument in arguments]
    qbaf_with = QBAFramework(arguments, initial_strengths,
                            attackers, supporters,
                            semantics=qbaf.semantics)
    qbaf_without = restrict(qbaf_with, qbaf.arguments - contributors)
    fs_with = qbaf_with.final_strengths[topic]
    fs_without = qbaf_without.final_strengths[topic]
    return fs_with - fs_without