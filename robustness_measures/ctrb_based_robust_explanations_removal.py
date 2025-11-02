from qbaf import QBAFramework
from qbaf_ctrbs. removal import determine_removal_ctrb


def return_difference ( explanation_1, explanation_2) :
  """Calculates the set-theoretic difference between the 
  lists explanation_1 and explanation_2

  Args:
      explanation_1 (list/dictionary): The first list of arguments.
      explanation_1 (list/dictionary): The first list of arguments.

  Returns:
      list: The set theoretic difference between arguments
  """

  arg_list = [ ]

  for x in explanation_1:

    if x not in explanation_2:
      arg_list. append ( x )

  for x in explanation_2:

    if x not in explanation_1:
      arg_list. append ( x )

  return list (dict. fromkeys ( arg_list ) )



def influence_towards_inferences ( qbaf_1, qbaf_2, arg_list, inference_1, inference_2 ) :
  """Calculates the influence of the arguments in arg_list 
  towards the inferences 1 and 2. The influence is measured using contribution functions;
  the contribution function used here is removal based. 

  Args:
      qbaf_1 (QBAFramewrok): The initial QBAF.
      qbaf_2 (QBAFramework): The updated QBAF.
      arg_list (list): List of arguments whose influences would be measured.
      inference_1 (string): The first inference towards which the influence would be measured.
      influence_2 (string): The second inference towards which the influence would be measured. 

  Returns:
      float: The influence arg_list has towards the inferences, computed as a sum of individual contributions.
  """

  sum = 0.0

  for x in arg_list :

    if x in qbaf_1. arguments :
      sum = sum + determine_removal_ctrb ( x, inference_1, qbaf_1 ) + determine_removal_ctrb ( x, inference_2, qbaf_1 )

    else :
      sum = sum + determine_removal_ctrb ( x, inference_1, qbaf_2 ) + determine_removal_ctrb ( x, inference_2, qbaf_2 )

  return sum



def check_ctrb_based_robust_SSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :
  """checks whether an explanation is contribution based sufficiently robust w.r.t. 
     qbaf, epsilon and qbaf_collection. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): a list of QBAF Updates.
      explanation (list/dictionary): A subset of arguments.
      epsilon (list): The degree of tolerance for any explanation'a modification.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """


  for x in qbaf_collection :

    c = 0

    for y in qbaf. minimalSSIExplanations ( x, inference_1, inference_2 ) :

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





def check_ctrb_based_robust_CSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :
    """checks whether an explanation is contribution based counterfactually robust w.r.t. 
      qbaf, epsilon and qbaf_collection. 

    Args:
      qbaf (QBAFramewrok): The initial QBAF.
      explanation (list/dictionary): A subset of arguments.
      qbaf_collection (list): a list of QBAF Updates.
      epsilon (float): The degree of tolerance for any explanation'a modification.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

    Returns:
      bool: The final truth value of the above described check.
    """

    for x in qbaf_collection :

        c = 0

        for y in qbaf. minimalCSIExplanations ( x, inference_1, inference_2 ) :

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




def check_ctrb_based_robust_NSI_explanations ( qbaf, explanation, epsilon, qbaf_collection, inference_1, inference_2 ) :
  """checks whether an explanation is contribution based necessarily robust w.r.t. 
     qbaf, epsilon and qbaf_collection. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): a list of QBAF Updates.
      explanation (list/dictionary): A subset of arguments.
      epsilon (float): The degree of tolerance for any explanation'a modification.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """

  for x in qbaf_collection :

    c = 0

    for y in qbaf. minimalNSIExplanations ( x, inference_1, inference_2 ) :

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
