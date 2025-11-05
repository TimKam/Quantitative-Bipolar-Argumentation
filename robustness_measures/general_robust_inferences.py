

from qbaf import QBAFramework 

def check_general_robust_inferences ( qbaf_0, QBAFUpdates, a, b ):
  """checks general robustness of inferences w.r.t. QBAFUpdates

  Args:
      qbaf_0 (QBAFramewrok): The initial QBAF.
      QBAFUpdates (list): A list of updated QBAFs.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """

  for item in QBAFUpdates:

    if ( qbaf_0. are_strength_consistent( item, a, b ) == False ):
      return False

  return True


