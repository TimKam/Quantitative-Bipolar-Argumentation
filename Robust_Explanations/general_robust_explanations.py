from qbaf import QBAFrameworks, QBAFRelations

"""
 Function to check whether an 'explanation' is robust over the collection
 of qbafs 'qbaf_collection', i.e. it checks whether 'explanation' is a SSI
 explanation for each change of inference from 'qbaf' to x in 'qbaf_collection'.
"""

def check_robust_SSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):

  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True



"""
 Function to check whether an 'explanation' is robust over the collection
 of qbafs 'qbaf_collection', i.e. it checks whether 'explanation' is a CSI
 explanation for each change of inference from 'qbaf' to x in 'qbaf_collection'.
"""

def check_robust_CSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):

  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalCSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True



"""
 Function to check whether an 'explanation' is robust over the collection
 of qbafs 'qbaf_collection', i.e. it checks whether 'explanation' is a SSI
 explanation for each change of inference from 'qbaf' to x in 'qbaf_collection'.
"""

def check_robust_NSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):

  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalNSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True
