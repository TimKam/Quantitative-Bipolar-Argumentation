from qbaf import QBAFramework
from itertools import combinations

def new_arguments(initial_qbaf: QBAFramework,
                  qbaf_collection: list[QBAFramework]) -> set[str]:

    """
    Returns the set of new arguments that gets added to qbaf_initial by qbaf_collection.

    Args:
       initial_qbaf (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.

    Returns:
       set[str]: The set of new arguments that gets added to qbaf_initial.
    """
    new_args = set()
    args = initial_qbaf.arguments

    for qbaf in qbaf_collection:
      new_args_for_qbaf = qbaf.arguments.difference(args)
      for arg in new_args_for_qbaf:
        new_args.add(arg)

    return new_args




def subsets_of_arguments(args: set[str]) -> list[set[str]]:

  n = len(args)
  subset_of_args = []

  for i in range (0, n + 1):
    list_of_args = combinations(list(args), i)
    for subset in list_of_args:
      subset_of_args.append(set(subset))

  return subset_of_args




def is_pocket(qbaf_initial: QBAFramework,
              qbaf_collection: list[QBAFramework],
              pocket: list[str],
              topic_argument_1: str,
              topic_argument_2: str) -> bool:
    """
    Checks whether qbaf_initial is a pocket of qbaf_collection.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
       pocket (list[str]): The arguments we check whether is a pocket.
       topic_argument_1 (str): The first topic argument whose strength is  considered.
       topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
       bool: True if pocket is a pocket w.r.t. qbaf_initial and qbaf_collection, False otherwise.
    """
    assert topic_argument_1 in qbaf_initial.arguments
    assert topic_argument_2 in qbaf_initial.arguments

    subsets = subsets_of_arguments(set(pocket))

    for subset in subsets:
      s = subset.union(qbaf_initial.arguments)
      for qbaf in qbaf_collection:
        if (qbaf.arguments == s and
            qbaf_initial.are_strength_consistent(qbaf,
                                                 topic_argument_1,
                                                 topic_argument_2)):
              return False

    return True




def pockets_of_consistency(qbaf_initial: QBAFramework,
                           qbaf_collection: list[QBAFramework],
                           topic_argument_1: str,
                           topic_argument_2: str) -> list[set[str]]:
    """
    Retruns the list of pockets of qbaf_initial and qbaf_collection.

    Args:
       qbaf_initial (QBAFramework): The initial QBAF.
       qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
       topic_argument_1 (str): The first topic argument whose strength is  considered.
       topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
       list[set[str]]: The list of pockets.
    """
    new_args = new_arguments(qbaf_initial, qbaf_collection)
    p = []

    subset_of_new_args = subsets_of_arguments(new_args)

    for subset in subset_of_new_args:
      if(is_pocket(qbaf_initial,
                   qbaf_collection, subset,
                   topic_argument_1,topic_argument_2) == True): 
          p.append(subset)

    return p




def explanations_of_updates(qbaf_initial: QBAFramework,
                            qbaf_collection: list[QBAFramework],
                            topic_argument_1: str,
                            topic_argument_2: str) -> list[dict[str]]:
    """
    Returns the list of minimal arguments in qbaf_collection that explain the updates.

    Args:
      qbaf_initial (QBAFramework): The initial QBAF.
      qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
      topic_argument_1 (str): The first topic argument whose strength is  considered.
      topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
      list[dict[str]]: The list of minimal arguments in qbaf_collection that explain the updates.
    """
    pockets = pockets_of_consistency(qbaf_initial, qbaf_collection,
                                     topic_argument_1, topic_argument_2)
    new_args = new_arguments(qbaf_initial, qbaf_collection)
    explanations = dict()

    for pocket in pockets:
      n = new_args.minus(pocket)
      explanations.update({pocket: n})

    return explanations

