from qbaf import QBAFramework

def is_bounded_update(qbaf_initial: QBAFramework,
                      qbaf_update: QBAFramework,
                      epsilon: float,
                      mutable_args: set) -> bool:
    """
    Checks whether qbaf_update is an epsilon and mutable_args bounded update.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_update (QBAFramework): The updated QBAF.
       mutable_args (set): The set of mutable arguments whose strength is suspected to change.
       epsilon (float): The permissible range of strength change.

    Retruns:
      bool: True if qbaf_update is an epsilon and mutable_args bounded update,false otherwise.
    """
    k = 0

    if (qbaf_initial.arguments == qbaf_update.arguments and
        qbaf_initial.attack_relations == qbaf_update.attack_relations and
        qbaf_initial.support_relations == qbaf_update.support_relations):
          k = 1

    if (k == 1):

          for arg in mutable_args:
            if ((qbaf_update.initial_strength(arg)
                >= (qbaf_initial.initial_strength(arg) + 0.5)) or
                (qbaf_update.initial_strength(arg)
                <= (qbaf_initial.initial_strength(arg) - 0.5))
                ):
                  return False

          for arg in qbaf_initial.arguments.difference(mutable_args):
            if (qbaf_update.initial_strength(arg)
                != qbaf_initial.initial_strength(arg)):
                  return False

          return True

    if (k == 0):  return False




def is_expansion(qbaf_initial: QBAFramework,
                 qbaf_update: QBAFramework) -> bool:
    """
    Checks whether qbaf_update is a normal expansion of qbaf_initial.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_update (QBAFramework): The updated QBAF.

    Returns:
       bool: True if qbaf_update is a normal expansion of qbaf_initial, False otherwise.
    """
    k = 0
    args_initial = qbaf_initial.arguments[:]
    attack_rel = qbaf_initial.attack_relations[:]
    support_rel = qbaf_initial.support_relations[:]

    if (args_initial.issubset(qbaf_update.arguments) and
        attack_rel.issubset(qbaf_update.attack_relations) and
        support_rel.issubset(qbaf_update.support_relations)):
          k = 1

    if (k == 1):
          args = qbaf_initial.arguments

          for arg in args:
            if (qbaf_initial.initial_strength(arg)
                != qbaf_update.initial_strength(arg)):
                  return False

          return True

    if (k == 0):  return False




def general_robust_consistent(qbaf_initial: QBAFramework,
                              qbaf_collection: list[QBAFramework],
                              topic_argument_1: str,
                              topic_argument_2: str) -> bool:
    """
    Checks whether qbaf_initial is general robust consistent with respect to
    qbaf_collection.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
       topic_argument_1 (str): The first topic argument whose strength is  considered.
       topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
       bool: True if qbaf_initial is general robust consistent with respect to
       qbaf_collection, False otherwise.
    """

    for qbaf in qbaf_collection:
      if (not qbaf.are_strength_consistent(qbaf_initial,
                                           topic_argument_1,
                                           topic_argument_2)):
        return False

    return True




def expansion_robust_consistent(qbaf_initial: QBAFramework,
                                 qbaf_collection: list[QBAFramework],
                                 topic_argument_1: str,
                                 topic_argument_2: str ) -> bool:
    """
    Checks whether qbaf_initial is expansion robust consistent with respect to
    qbaf_collection.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
       topic_argument_1 (str): The first topic argument whose strength is  considered.
       topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
       bool: True if qbaf_initial is general robust consistent with respect to
       qbaf_collection, False otherwise.
    """
    updated_qbaf_collection = [qbaf for qbaf in qbaf_collection if (is_expansion(qbaf_initial, qbaf))]

    for qbaf in updated_qbaf_collection:
      if (not qbaf.are_strength_consistent(qbaf_initial,
                                           topic_argument_1,     
                                           topic_argument_2)):
        return False

    return True




def bounded_update_robust_consistent(qbaf_initial: QBAFramework,
                                     qbaf_updates: list[QBAFramework],
                                     topic_argument_1: str,
                                     topic_argument_2: str,
                                     epsilon: float,
                                     mutable_args: set) -> bool:
    """
    Checks whether qbaf_initial is bounded-update robust consistent with respect to qbaf_collection, epsilon and the mutable_args.

    Args:
      qbaf_initial (QBAFramework): The initial QBAF.
      qbaf_updates (list[QBAFramework]): The collection of QBAF updates.
      topic_argument_1 (str): The first topic argument whose strength is  considered.
      topic_argument_2 (str): The second topic argument whose strength is  considered.
      epsilon (float): The permissible range of strength change.
      mutable_args (set): The set of mutable arguments whose strength is suspected to change.

    Returns:
      bool: True if qbaf_initial is bounded-update robust consistent with respect to qbaf_collection.
    """
    updated_qbaf_collection = [qbaf for qbaf in qbaf_updates if (is_bounded_update(qbaf_initial, qbaf, epsilon, mutable_args))]

    for qbaf in updated_qbaf_collection:
      if (not qbaf.are_strength_consistent(qbaf_initial,
                                           topic_argument_1,
                                           topic_argument_2)):
        return False

    return True