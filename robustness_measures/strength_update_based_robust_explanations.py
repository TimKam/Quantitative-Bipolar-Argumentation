from qbaf import QBAFramework
from robustness_measures.strength_update_based_robust_inferences import *



def check_strength_update_based_SSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is strength updated based sufficiently robust w.r.t. 
     qbaf, argstar and epsilon. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      argstar (list): The list of arguments which are susceptible to initial strength changes. 
      epsilon (float): The tolerance of initial strength change for qbaf.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True



def check_strength_update_based_CSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is strength updated based counter-factually robust w.r.t. 
     qbaf, argstar and epsilon. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      argstar (list): The list of arguments which are susceptible to initial strength changes. 
      epsilon (float): The tolerance of initial strength change for qbaf.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True




def check_strength_update_based_NSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is strength updated based necessarily robust w.r.t. 
     qbaf, argstar and epsilon. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      argstar (list): The list of arguments which are susceptible to initial strength changes. 
      epsilon (float): The tolerance of initial strength change for qbaf.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True

