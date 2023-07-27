from qbaf import QBAFramework

def determine_gradient_ctrb(topic, contributor, qbaf, epsilon=1.4901161193847656e-08):
    """Determines the gradient contribution of a contributor
    to a topic argument.

    Args:
        topic (string): The topic argument
        contributor (string): The contributing argument
        qbaf (QBAFramework): The QBAF that contains topic and contributor
        epsilon (float): Epsilon used by the approximator. Defaults to 1.4901161193847656e-08.

    Returns:
        float: The contribution of the contributor to the topic
    """
    if not topic in qbaf.arguments or not contributor in qbaf.arguments:
        raise Exception ('Topic and contributor must be in the QBAF.')
    
    def func(contributor_strength, qbaf):
        initial_strengths = []
        argument_list = list(qbaf.arguments)
        for arg in argument_list:
            if arg == contributor:
                initial_strengths.append(contributor_strength)
            else:
                initial_strengths.append(qbaf.initial_strength(arg))
        qbaf_changed = QBAFramework(argument_list, initial_strengths,
                                    qbaf.attack_relations.relations,
                                    qbaf.support_relations.relations,
                                    semantics=qbaf.semantics)
        return qbaf_changed.final_strength(topic)
    
    contributor_strength = qbaf.initial_strength(contributor)
    strength_base = func(contributor_strength, qbaf)
    try:
        strength_epsilon = func(contributor_strength + epsilon, qbaf)
        return (strength_epsilon - strength_base) / epsilon
    except ValueError:
        strength_epsilon = func(contributor_strength - epsilon, qbaf)
        return (strength_base - strength_epsilon) / epsilon