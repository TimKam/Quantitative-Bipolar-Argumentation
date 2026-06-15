import copy
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
       list[set[str]]/bool: Returns the set of all sub-pockets, if pocket is a pocket of consistency; False otherwise.
    """
    assert topic_argument_1 in qbaf_initial.arguments
    assert topic_argument_1 in qbaf_initial.arguments


    if(not pocket): return [set()]

    pocket_copy = list(copy.deepcopy(pocket))
    subset_of_pockets = list()
    y = pocket_copy[0]
    pocket_wo_y = [x for x in pocket_copy if x!= y]

    val = is_pocket(qbaf_initial,
                    qbaf_collection,
                    pocket_wo_y,
                    topic_argument_1,
                    topic_argument_2)     #Recursive call to is_pocket



    if(val == False):
        return False


    for x in val:
      A = x.union({y}) 
      potential_args = A.union(qbaf_initial.arguments)
      for h in qbaf_collection:
        if(h.arguments == set(potential_args) and
           h.are_strength_consistent(qbaf_initial, topic_argument_1, topic_argument_2) == False):
              return False
      subset_of_pockets.append(A)

    return(subset_of_pockets + val)



def is_pocket_max(qbaf_initial: QBAFramework,
                  qbaf_collection: list[QBAFramework],
                  pocket: set[str],
                  topic_argument_1: str,
                  topic_argument_2:str)-> bool:
   
   """
   Checks whether pocket is a pocket of consistency with respect to qbaf_initial and qbaf_collection as per the subset relation.

   Args: 
      qbaf_initial (QBAFramework): The initial QBAF. 
      qbaf_collection (list[QBAFramework]): The collection of QBAFs to be considered.
      topic_argument_1 (str): The first topic argument to be considered.
      topic_argument_2 (str): The second topic argument to be considered.

   Returns:
      bool: Returns True if pocket is a pocket of consistency; False otherwise.  
   """

   qbaf_collection_mdf = []

   for x in qbaf_collection:
      args = new_arguments(qbaf_initial, [x])
      if args.issubset(set(pocket)): 
         qbaf_collection_mdf.append(x)
   
   
   for qbaf in qbaf_collection_mdf:
      args = set(qbaf.arguments)
      if((qbaf_initial.are_strength_consistent(qbaf, 
                                                topic_argument_1, 
                                                topic_argument_2) == False)):
         return False
      
   return True



def max_pockets(qbaf_initial: QBAFramework,
                  qbaf_collection: list[QBAFramework],
                  pocket: set[str],
                  topic_argument_1: str,
                  topic_argument_2:str)-> set[list[str]]:
   """
   Returns the set of all maximal pockets with respect to qbaf_initial and qbaf_collection.
   
   Args: 
      qbaf_initial (QBAFramework): The initial QBAF. 
      qbaf_collection (list[QBAFramework]): The collection of QBAFs to be considered.
      topic_argument_1 (str): The first topic argument to be considered.
      topic_argument_2 (str): The second topic argument to be considered.

   Returns:
      set[list[str]]: returns the set of all maximal pockets with respect to qbaf_intial and qbaf-collection.   
   """

   pckt = copy.deepcopy(pocket)  

   if (is_pocket_max(qbaf_initial, qbaf_collection, pckt, topic_argument_1, topic_argument_2)): 
      return [pocket]
   
   nxt_itr = combinations(pckt, len(pckt)-1)
   max_pkts = list()

   for subsets in nxt_itr:
      max_pkts= max_pkts + max_pockets(qbaf_initial, 
                                       qbaf_collection, 
                                       list(subsets),
                                       topic_argument_1, topic_argument_2)
   
      
   return max_pkts





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
    pockets = []

    for i in range(len(new_args), 0, -1):
       combinations_of_args = combinations(new_args, i)
       for subset in combinations_of_args:
          fl = 0
          subset_transform = set(subset)
          for x in pockets:
             if(subset_transform.issubset(x)):  fl = 1
          
          l = copy.deepcopy(list(subset))
          if(fl == 0 and
             is_pocket(qbaf_initial,
                       qbaf_collection, l,
                       topic_argument_1,topic_argument_2) != False): 
              pockets.append(subset_transform) 

    return pockets




def explanation_of_robustness_violation(qbaf_initial: QBAFramework,
                            qbaf_collection: list[QBAFramework],
                            topic_argument_1: str,
                            topic_argument_2: str) -> list[tuple[str]]:
    """
    Returns the list of minimal arguments in qbaf_collection that explain the updates.

    Args:
      qbaf_initial (QBAFramework): The initial QBAF.
      qbaf_collection (list[QBAFramework]): The collection of QBAF updates.
      topic_argument_1 (str): The first topic argument whose strength is  considered.
      topic_argument_2 (str): The second topic argument whose strength is  considered.

    Returns:
      list[tuple[str]]: The list of minimal arguments in qbaf_collection that explain the updates.
    """
    pockets = pockets_of_consistency(qbaf_initial, qbaf_collection,
                                     topic_argument_1, topic_argument_2)
    new_args = new_arguments(qbaf_initial, qbaf_collection)
    explanations = []

    if(new_args in pockets):
      p = [(new_args, set())]
      return p

    for pocket in pockets:
      n = new_args.difference(pocket)
      for x in n:
        explanations.append((pocket, x))

    return explanations
