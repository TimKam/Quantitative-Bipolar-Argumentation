from robustness_measures. general_robust_inferences import check_general_robust_inferences
from qbaf import QBAFramework
from itertools import product

"""
 
"""

def subsetRec ( given_list ) :
  """Generates all the sublists of a given list;
     Used in generating the sub-QBAFs.

  Args:
      given_list (list) : A list of arguments

  Returns:
      list: The list of all subsets of given_list
  """

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




def Is_subset ( sublist, suplist ): 
  """check the elements of a list/dictonary 'sublist' is 
     present in 'suplist'.

  Args:
      sublist (list/dictonary) : A list of arguments.
      suplist (list/dictonary) : A list of arguments.

  Returns:
      bool: Returns true iff sublist is a subset of suplist, false otherwise.
  """



  for x in sublist: 

    if x not in suplist:
      return False

  return True




def generate_subqbaf_arguments ( qbaf_initial, qbaf_final ) :
  """considers all subsets of arguments in 'qbaf_final'
     and returns the subsets which are superset of 'qbaf_inital'.

  Args:
      qbaf_initial (QBAFramework) : The initial QBAF.
      qbaf_final (QBAFramework) : The final QBAF, qbaf_initial being a sub-QBAF of it.

  Returns:
      list: Returns the subsets of arguments of qbaf_final, which has qbaf_initial's arguments as subarguments.
  """



  arguments = qbaf_final. arguments

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

"""

def generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final ) :
  """ Calculates the expansions of 'qbaf_inital' which are sub-QBAFs of
      qbaf_final.

  Args:
      qbaf_initial (QBAFramework) : The initial QBAF.
      qbaf_final (QBAFramework) : The final QBAF, qbaf_initial being a sub-QBAF of it.

  Returns:
      list: Returns a list of sub-QBAFs of qbaf_final which have qbaf_initial as a sub-QBAF.
  """

  arguments_list = generate_subqbaf_arguments ( qbaf_initial, qbaf_final )
  qbaf_collection = [ ]


  for x in arguments_list :

    args_G = x

    atts_G = []
    supps_G = []
    initial_strength_G = []


    p = product (x, repeat = 2)

    for i in p :

      if ( i in qbaf_final. attack_relations ) : 
        atts_G. append ( i )

      if ( i in qbaf_final. support_relations ) : 
        supps_G. append ( i )
    
    for i in args_G:
      initial_strength_G.append ( qbaf_final.initial_strengths [ i ] )

    qbaf_collection.append ( QBAFramework ( args_G, initial_strength_G, atts_G, supps_G ) )

  return qbaf_collection

"""
 Function to check expanison based robustness holds or not.
"""

def check_expansion_based_robust_inferences ( qbaf_initial, qbaf_final, inference_1, inference_2 ) :
  """ checks whether inferences are expansion based robust or not.

  Args:
      qbaf_initial (QBAFramework) : The initial QBAF.
      qbaf_final (QBAFramework) : The final QBAF, qbaf_initial being a sub-QBAF of it.
      inference_1 (string): The first inference.
      inference_2 (string): The second inference.

  Returns:
      bool: Returns the result of the above check.
  """


  qbaf_collection = generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final )

  qbaf_collection. remove ( qbaf_final )

  return check_general_robust_inferences ( qbaf_initial, qbaf_collection, inference_1, inference_2 )