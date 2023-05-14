from qbaf_ctrbs.utils import restrict

def determine_removal_ctrb(topic, contributor, qbaf):
    """Determines the removal contribution of a contributor
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
            'An argument\'s removal contribution to itself cannot be determined.')
    if not topic in qbaf.arguments or not contributor in qbaf.arguments:
        raise Exception ('Topic and contributor must be in the QBAF.')
    fs_with = qbaf.final_strengths[topic]
    restriction = restrict(qbaf, qbaf.arguments - {contributor})
    fs_without = restriction.final_strengths[topic]
    return fs_with - fs_without 