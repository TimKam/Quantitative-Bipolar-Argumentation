from qbaf_ctrbs.utils import restrict

def determine_removal_ctrb(topic, contributors, qbaf):
    """Determines the removal contribution of a contributor
    to a topic argument.

    Args:
        topic (string): The topic argument
        contributors (string or set): The contributing argument(s)
        qbaf (QBAFramework): The QBAF that contains topic and contributor

    Returns:
        float: The contribution of the contributor to the topic
    """
    if topic in contributors:
        raise Exception (
            'An argument\'s removal contribution to itself cannot be determined.')
    # Ensure topic and argument(s) are in QBAF.
    if not all(item in qbaf.arguments for item in [topic, *contributors]):
            raise Exception ('Topic and contributor must be in the QBAF.')
    # Check if input is a set of arguments (type: set) or single argument (type: string).
    if not isinstance(contributors, set):
        contributors = {contributors}
    fs_with = qbaf.final_strengths[topic]
    restriction = restrict(qbaf, qbaf.arguments - contributors)
    fs_without = restriction.final_strengths[topic]
    return  fs_with - fs_without