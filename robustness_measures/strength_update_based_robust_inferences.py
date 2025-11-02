import numpy as np
from itertools import product

from qbaf import QBAFramework, QBAFARelations
from robustness_measures. general_robust_inferences import check_general_robust_inferences



def generate_strength_update_values ( qbaf, argstar, epsilon ):
  """creates a list of dictornaries:
     for each dictonary, the keys are elements of 'argstar' and
     the corresponding values are a number between (-'epsilon', +'epsilon')

  Args:
      qbaf (QBAFramewrok): The considered QBAF.
      argstar (list/dictionary): A list of arguments, which are a subset of qbaf's arguments.
      epsilon (float): The degree of change for the initial strength of argstar.

  Returns:
      list: Returns a list of dictonaries with the description given above.
  """

  values = [ ]
  values_for_argstar = [ ]


  for x in np.arange ( -float( epsilon ), float( epsilon ), 0.01 ):
      values. append ( str ( round ( x, 3 ) ) )


  n = len ( argstar )
  values_buffer = product ( values, repeat = n )


  for x in values_buffer :

    dictionary_buffer = {}
    l = 0

    for y in argstar :
      dictionary_buffer. update ( { y : x[l] } )
      l += 1

    values_for_argstar.append ( dictionary_buffer )


  return values_for_argstar





def generate_strength_update( qbaf, args, epsilon ) :
  """creates a QBAFramework that changes the values of initial strength by epsilon 
     for each argument in args and in args 

  Args:
      qbaf (QBAFramewrok): The considered QBAF.
      args (list/dictionary): A list of arguments, which are a subset of qbaf's arguments.
      epsilon (float): The change for the initial strength of argstar.

  Returns:
      QBAFramework: Returns a strength upated QBAF.
  """


  args_G = [ ]
  initial_strength_G = [ ]
  atts_G = [ ]
  supps_G = [ ]

  for x in qbaf. arguments :

    args_G. append ( x )

    if x in args :
      initial_strength_G. append ( qbaf. initial_strengths[ x ] + float ( epsilon[ x ] ) )

    else:
      initial_strength_G. append ( qbaf. initial_strengths[ x ] )

  for x in qbaf. attack_relations :
    atts_G. append ( x )

  for x in qbaf. support_relations :
    supps_G. append ( x )


  return QBAFramework ( args_G, initial_strength_G, atts_G, supps_G )


def generate_strength_update_collection ( qbaf, argstar, epsilon ) :
  """uses 'generate_strength_update' and 'generate_strength_values'
     to generate the  (initial) strength update based collection for a given 'qbaf'

  Args:
      qbaf (QBAFramewrok): The considered QBAF.
      argstar (list/dictionary): A list of arguments, which are a subset of qbaf's arguments.
      epsilon (float): The degree of change for the initial strength of argstar.

  Returns:
      list: Returns a list of strength updated QBAFs.
  """


  values = generate_strength_update_values ( qbaf, argstar, epsilon )
  qbaf_collection = [ ]

  for x in values :
    qbaf_collection.append ( generate_strength_update ( qbaf, argstar, x ) )

  return qbaf_collection




def check_update_based_robust_inferences ( qbaf, argstar, epsilon, inference_1, inference_2 ) :
  """Creates a collection 'G' of QBAF updates using 'qbaf', 'argstar' and 'epsilon'
     and checks for robustness w.r.t.  members of 'G' and arguments 'inference_1' and 'inference_2'

  Args:
      qbaf (QBAFramewrok): The considered QBAF.
      argstar (list/dictionary): A list of arguments, which are a subset of qbaf's arguments.
      epsilon (float): The degree of change for the initial strength of argstar.
      inference_1 (string): The first inference.
      inference_2 (string): The second inference.

  Returns:
      list: Returns a list of strength updated QBAFs.
  """


  G = generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in G:

    if ( qbaf.are_strength_consistent ( x, str ( inference_1 ), str ( inference_2 ) ) == False ) :
      return False

  return True