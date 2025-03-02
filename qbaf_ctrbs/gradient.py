from qbaf import QBAFramework

def determine_gradient_ctrb(topic, contributors, qbaf, epsilon=1.4901161193847656e-08, aggregation_fn=max):
    """Determines the gradient contribution of a contributor
    or a set of contributors to a topic argument.

    Args:
        topic (string): The topic argument
        contributors (string or set): The contributing argument(s)
        qbaf (QBAFramework): The QBAF that contains topic and contributor
        epsilon (float): Epsilon used by the approximator. Defaults to 1.4901161193847656e-08.
        aggregation_fn (function): Function to aggregate gradient contributions. Defaults to max.

    Returns:
        float: The contribution of the contributor to the topic
    """
    if not isinstance(contributors, set):
        contributors = {contributors}
    if not all(item in qbaf.arguments for item in [topic, *contributors]):
            raise Exception ('Topic and contributor must be in the QBAF.')
    
    def func(contributor, contributor_strength, qbaf):
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
    
    all_contributors_strength = [(contributor, qbaf.initial_strength(contributor)) for contributor in contributors]

    attribution_list = []
    for contributor, contributor_strength in all_contributors_strength:
        strength_base = func(contributor, contributor_strength, qbaf)
        try:
            strength_epsilon = func(contributor, contributor_strength + epsilon, qbaf)
            attribution_list.append(((strength_epsilon - strength_base) / epsilon))
        except ValueError:
            strength_epsilon = func(contributor, contributor_strength - epsilon, qbaf)
            attribution_list.append(((strength_base - strength_epsilon) / epsilon))    
    return aggregation_fn(attribution_list)