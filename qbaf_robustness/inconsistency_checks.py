from qbaf import QBAFramework
from qbaf_robustness.consistency_checks import is_expansion,is_bounded_update



def is_general_robust_inconsistent(qbaf_initial: QBAFramework,
                              qbaf_collection: list[QBAFramework],
                              topic_argument_1: str,
                              topic_argument_2: str) -> bool:


    """
    Checks whether qbaf_initial is general robust inconsistent with respect to
    qbaf_collection.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
       topic_argument_1 (str): The first topic argument whose strength is  considered.
       topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
       bool: True if qbaf_initial is general robust inconsistent with respect to
       qbaf_collection, False otherwise.
    """

    for qbaf in qbaf_collection:
      if (qbaf.are_strength_consistent(qbaf_initial,
                                       topic_argument_1,
                                       topic_argument_2)):
        return False

    return True



def is_expansion_robust_inconsistent( qbaf_initial: QBAFramework,
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
    
    updated_qbaf_collection = [qbaf for qbaf in qbaf_collection if (is_expansion (qbaf_initial, qbaf))]

    for qbaf in updated_qbaf_collection:
      if (not qbaf.are_strength_consistent(qbaf_initial,
                                       topic_argument_1,
                                       topic_argument_2)):
            return False

    return True



def is_bounded_updates_robust_inconsistent(qbaf_initial: QBAFramework,
                                     qbaf_updates: list[QBAFramework],
                                     topic_argument_1: str,
                                     topic_argument_2: str,
                                     epsilon: float,
                                     mutable_args: set) -> bool:
    """
    Checks whether qbaf_initial is bounded-update robust inconsistent with respect to qbaf_collection, epsilon and the mutable_args.

    Args:
      qbaf_initial (QBAFramework): The initial QBAF.
      qbaf_updates (list[QBAFramework]): The collection of QBAF updates.
      topic_argument_1 (str): The first topic argument whose strength is  considered.
      topic_argument_2 (str): The second topic argument whose strength is  considered.
      epsilon (float): The permissible range of strength change.
      mutable_args (set): The set of mutable arguments whose strength is suspected to change.

    Returns:
      bool: True if qbaf_initial is bounded-update robust inconsistent with respect to qbaf_collection, False otherwise.
    """
    updated_qbaf_collection = [qbaf for qbaf in qbaf_updates if (is_bounded_update(qbaf_initial, qbaf, epsilon, mutable_args))]

    for qbaf in updated_qbaf_collection:
      if (qbaf.are_strength_consistent(qbaf_initial,
                                           topic_argument_1,
                                           topic_argument_2)):
        return False

    return True