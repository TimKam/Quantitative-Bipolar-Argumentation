from qbaf import QBAFramework
from qbaf_ctrbs.utils import restrict

def determine_iremoval_ctrb(topic, contributor, qbaf):
    """Determines the intrinsic removal contribution of a contributor
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
            'An argument\'s intrinsic removal contribution to itself cannot be determined.')
    if not topic in qbaf.arguments or not contributor in qbaf.arguments:
        raise Exception ('Topic and contributor must be in the QBAF.')
    attackers = [(source, target) for source, target in qbaf.attack_relations.relations if not target == contributor]
    supporters = [(source, target) for source, target in qbaf.support_relations.relations if not target == contributor]
    arguments = list(qbaf.arguments)
    initial_strengths = [qbaf.initial_strengths[argument] for argument in arguments]
    qbaf_with = QBAFramework(arguments, initial_strengths,
                            attackers, supporters,
                            semantics=qbaf.semantics)
    qbaf_without = restrict(qbaf_with, qbaf.arguments - {contributor})
    fs_with = qbaf_with.final_strengths[topic]
    fs_without = qbaf_without.final_strengths[topic]
    return fs_with - fs_without