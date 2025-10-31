from qbaf import QBAFrameworks, QBAFRelations
from qbaf_ctrbs. removal import determine_removal_ctrb


"""
 Returns the difference between the lists 'explanation_1' and 'explanation_2'.
"""

def return_difference ( explanation_1, explanation_2) :

  arg_list = [ ]

  for x in explanation_1:

    if x not in explanation_2:
      arg_list. append ( x )

  for x in explanation_2:

    if x not in explanation_1:
      arg_list. append ( x )

  return list (dict. fromkeys ( arg_list ) )



"""
 Function that checks the contribution of each argument in 'arg_list'
 towards 'inference_1' and 'inference_2'.
"""

def influence_towards_inferences ( qbaf_1, qbaf_2, arg_list, inference_1, inference_2 ) :

  sum = 0

  for x in arg_list :

    if x in qbaf_1. arguments :
      sum = sum + determine_removal_ctrb ( x, inference_1, qbaf_1 ) + determine_removal_ctrb ( x, inference_2, qbaf_1 )

    else :
      sum = sum + determine_removal_ctrb ( x, inference_1, qbaf_2 ) + determine_removal_ctrb ( x, inference_2, qbaf_2 )

  return sum

"""
 Function that checks whether a given explanation is contribution based Sufficiently Robust or not,
 w.r.t. to 'qbaf', a collection 'qbaf_collection', a subset of arguments 'explanation' and the inferences
 'inference_1' and 'inference_2'.
"""

def check_ctrb_based_robust_SSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :

  for x in qbaf_collection :

    c = 0

    for y in qbaf. minialSSIExplanations ( x, inference_1, inference_2 ) :

        arg_list = return_difference ( y, explanation )
        sum = influence_towards_inferences ( qbaf, x, arg_list, inference_1, inference_2 )

        if ( sum <= epsilon ) :
            c = c + 1
            break

        if  c == 0 :
            return False

        else:
            pass

  return True



"""
 Function that checks whether a given explanation is contribution based CounterFactually Robust or not,
 w.r.t. to 'qbaf', a collection 'qbaf_collection', a subset of arguments 'explanation' and the inferences
 'inference_1' and 'inference_2'.
"""

def check_ctrb_based_robust_CSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :

    for x in qbaf_collection :

        c = 0

        for y in qbaf. minialCSIExplanations ( x, inference_1, inference_2 ) :

            arg_list = return_difference ( y, explanation )
            sum = influence_towards_inferences ( qbaf, x, arg_list, inference_1, inference_2 )

            if ( sum <= epsilon ) :
                c = c + 1
                break

        if  c == 0 :
            return False

        else:
            pass

    return True


"""
 Function that checks whether a given explanation is contribution based Necessarily Robust or not,
 w.r.t. to 'qbaf', a collection 'qbaf_collection', a subset of arguments 'explanation' and the inferences
 'inference_1' and 'inference_2'.
"""

def check_ctrb_based_robust_SSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :

  for x in qbaf_collection :

    c = 0

    for y in qbaf. minialNSIExplanations ( x, inference_1, inference_2 ) :

      arg_list = return_difference ( y, explanation )
      sum = influence_towards_inferences ( qbaf, x, arg_list, inference_1, inference_2 )

      if ( sum <= epsilon ) :
        c = c + 1
        break

    if  c == 0 :
      return False

    else:
      pass

  return True