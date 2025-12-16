from qbaf import QBAFramework
from robustness_checks.robust_consistency_checks import is_bounded_update
from robustness_checks.robust_consistency_checks import is_expansion

def general_robust_inconsistent(qbaf_initial: QBAFramework,
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



def expansion_robust_inconsistent( qbaf_initial: QBAFramework,
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
    
    updated_qbaf_collection = [qbaf for qbaf in qbaf_collection if (is_expansion(qbaf_initial, qbaf) == 1)]

    for qbaf in updated_qbaf_collection:
      if (not qbaf.are_strength_consistent(qbaf_initial,
                                       topic_argument_1,
                                       topic_argument_2)):
        for qbaf_alt in qbaf_collection:
          if (is_expansion(qbaf, qbaf_alt) and
              qbaf_alt.are_strength_consistent(qbaf,
                                               topic_argument_1,
                                               topic_argument_2)):
            return False

    return True



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

    if (qbaf_initial.arguments.issubset(qbaf_update.arguments) and
        qbaf_initial.attack_relations.issubset(qbaf_update.attack_relations) and
        qbaf_initial.support_relations.issubset(qbaf_update.support_relations)):
          k = 1

    if (k == 1):
          args = qbaf_initial.arguments

          for arg in args:
            if (qbaf_initial.initial_strength(arg)
                != qbaf_update.initial_strength(arg)):
                  return False

          return True

    if (k == 0):  return False



