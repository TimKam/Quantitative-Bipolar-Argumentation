from qbaf import QBAFramework, QBAFARelations

"""
 Function to check whether a QBAF is robustly inconsistence over the collection
 of qbafs 'qbaf_collection', i.e. it checks whether there is always a change of
 inference between qbaf and x in 'qbaf_collection'.
"""

def check_general_robustly_inconsistent ( qbaf, qbaf_collection, inference_1, inference_2 ) :

  for x in qbaf_collection :

    if ( qbaf. are_strength_consistent ( x, inference_1, inference_2 ) == True ) :
      return False

  return True