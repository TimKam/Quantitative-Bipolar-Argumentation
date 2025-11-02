from qbaf import QBAFramework, QBAFARelations
from robustness_measures.strength_update_based_robust_inferences import *
from robustness_measures.general_robustly_inconsistent import check_general_robustly_inconsistent



def check_update_based_robustly_inconsistent ( qbaf, argstar, epsilon, inference_1, inference_2 ) :
  """checks whether a QBAF is robustly inconsistent over the collection
     of qbafs of initial strength changes by at-most epsilon and w.r.t. the subset of arguments 
     argstar

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      argstar (list): A subset of arguments suceptible to strength change. 
      epsilon (float): The range of change for the initial strength in argstar.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """
 

  G = generate_strength_update_collection ( qbaf, argstar, epsilon )

  return check_general_robustly_inconsistent ( qbaf, G, inference_1, inference_2 )

