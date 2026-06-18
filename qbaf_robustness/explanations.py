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
   
   for qbaf in qbaf_collection:
      args = new_arguments(qbaf_initial, [qbaf])
      if args.issubset(set(pocket)): 
         if not qbaf_initial.are_strength_consistent(qbaf, topic_argument_1, topic_argument_2):
            return False
      
   return True



def determine_max_pockets(qbaf_initial: QBAFramework,
                            qbaf_collection: list[QBAFramework],
                            topic_argument_1: str,
                            topic_argument_2: str,
                            current_subset: set[str]= set(),
                            pocket_list: list[set[str]]= [],
                            depth = 0)-> list[set[str]]:

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

   if (depth == 0):
      considered_subset = set(new_arguments(qbaf_initial, qbaf_collection))
   else: 
      considered_subset = current_subset

   if(is_pocket_max(qbaf_initial,
                    qbaf_collection,
                    considered_subset,
                    topic_argument_1,
                    topic_argument_2)):
      return [considered_subset]

   

   next_itr = [set(x) for x in combinations(considered_subset, 
                                                     len(considered_subset)-1)]
   
   next_itr_modified = [x for x in next_itr if not any(x.issubset(y) 
                                                for y in pocket_list)]



   for subset in next_itr_modified:
      pocket_list = pocket_list + determine_max_pockets(qbaf_initial,
                                                          qbaf_collection,
                                                          topic_argument_1,
                                                          topic_argument_2,
                                                          subset,
                                                          pocket_list,
                                                          depth + 1)
      
   if depth != 0: 
      return pocket_list
   else:
      max_p = [x for x in pocket_list if (not any( (x != y and x.issubset(y)) for y in pocket_list))]
      unique_pockets = []
      
      for x in max_p:
        if x not in unique_pockets:
          unique_pockets.append(x)
      return unique_pockets



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


def determine_linear_max_pockets(qbaf_initial: QBAFramework,
                                 qbaf_collection: list[QBAFramework],
                                 topic_argument_1: str,
                                 topic_argument_2: str) -> list[list[str]]:

  """ 
  Determines the pockets when the qbaf_collection is a linear chain. 

  Args:
    qbaf_initial (QBAFramework): The initial QBAF. 
    qbaf_collection (list[QBAFramework]): The collection of QBAFs to be considered. 
    topic_argument_1 (str): The first topic argument to be considered.
    topic_argument_2 (str): The second topic argument to be considered. 
  
  Returns:
    set[list[str]]: returns the set of all maximal pockets with respect to qbaf_intial and qbaf-collection.
  """

  collection_index = 0
  new_args = list(new_arguments(qbaf_initial, qbaf_collection))

  while (collection_index < len(qbaf_collection)):
    if (qbaf_initial.are_strength_consistent(qbaf_collection[collection_index],
                                            topic_argument_1,
                                            topic_argument_2) == False): 
      break
    else: 
      collection_index = collection_index+1


  args = set(new_arguments(qbaf_initial,[qbaf_collection[collection_index]])) 
  p_set = [set(x) for x in combinations(new_args, len(new_args)-1)]
  pocket_list = [list(x) for x in p_set if (args.issubset(x) == False)] 
  return pocket_list