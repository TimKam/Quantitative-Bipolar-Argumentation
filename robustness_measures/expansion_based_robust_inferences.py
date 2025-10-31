from robustness_measures. general_robust_inferences import check_general_robust_inferences
from qbaf import QBAFramework
from itertools import product

"""
 Generates all the sublists of a given list.
 Used in generating the sub-QBAFs.
"""

def subsetRec ( given_list ) :

  list_final = [ ]


  if given_list :

    y = given_list [ 0 ]
    update = given_list

    update. remove ( y )
    list_final. append ( [ y ] )
    
    for x in ( subsetRec ( update ) )  :

      if x : 

         p = x [ : ]
         q = x [ : ]

         list_final. append ( p )

         q. append ( y )
         list_final.append ( q )

      else :
         list_final.append ( [ ] )
  
  else :
    list_final = [ [ ] ]

  return list_final



"""
 Function to check the elements of a list/dictonary 'sublist' is 
 present in 'suplist'.
"""

def Is_subset ( sublist, suplist ): 

  for x in sublist: 

    if x not in suplist:
      return False

  return True



"""
 This function considers all subsets of arguments in 'qbaf_final'
 and returns the subsets which are superset of 'qbaf_inital'.
"""  

def generate_subqbaf_arguments ( qbaf_initial, qbaf_final ) :

  arguments = qbaf_final. arguments

# 'arguments_list' is used to turn the dictory 'arguments' to a list.
# 'arguments_updated' is the list of subsets of arguments we return.

  arguments_list = [ ]
  arguments_updated = [ ]

  for x in arguments :
    arguments_list. append ( x ) 

  buffer = subsetRec ( arguments_list )

  for x in buffer : 

      if ( Is_subset ( qbaf_initial . arguments, x ) == True ) :
        arguments_updated. append ( x )

  return arguments_updated


"""
 This function returns the expansions of 'qbaf_inital' which are sub-QBAFs of
 qbaf_final.
"""

def generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final ) :

  arguments_list = generate_subqbaf_arguments ( qbaf_initial, qbaf_final )
  qbaf_collection = [ ]
 
    # this loop creates a expanded QBAF w.r.t. to each subset of arguments of 'qbaf_final'
    # considered in 'arguments_list'.


  for x in arguments_list :

    args_G = x

    atts_G = []
    supps_G = []
    initial_strength_G = []

    # 'p' is the set of all argument pairs.
    # We loop over all the pairs and include the ones in att, supp relation of 'qbaf_final'.

    p = product (x, repeat = 2)

    for i in p :

      if ( i in qbaf_final. attack_relations ) : 
        atts_G. append ( i )

      if ( i in qbaf_final. support_relations ) : 
        supps_G. append ( i )
    
    # this loop is to assign the initial strength to the QBAF, based on 'qbaf_final'.

    for i in args_G:
      initial_strength_G.append ( qbaf_final.initial_strengths [ i ] )

    qbaf_collection.append ( QBAFramework ( args_G, initial_strength_G, atts_G, supps_G ) )

  return qbaf_collection

"""
 Function to check expanison based robustness holds or not.
"""

def check_expansion_based_robust_inferences ( qbaf_initial, qbaf_final, inference_1, inference_2 ) :

  qbaf_collection = generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final )

  qbaf_collection. remove ( qbaf_final)

  return check_general_robust_inferences ( qbaf_initial, qbaf_collection, inference_1, inference_2  )