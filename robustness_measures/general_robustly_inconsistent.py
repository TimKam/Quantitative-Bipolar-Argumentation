from qbaf import QBAFramework, QBAFARelations



def check_general_robustly_inconsistent ( qbaf, qbaf_collection, inference_1, inference_2 ) :
  """check whether a QBAF is robustly inconsistent over the collection
     of qbafs qbaf_collection.

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): The list of QBAFs against which robustness is measured.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """
 

  for x in qbaf_collection :

    if ( qbaf. are_strength_consistent ( x, inference_1, inference_2 ) == True ) :
      return False

  return True