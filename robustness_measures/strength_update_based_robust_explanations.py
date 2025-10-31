from qbaf import QBAFramework, QBAFARelations
from robustness_measures.strength_update_based_robust_inferences import *


"""
 Function that checks whether 'explnation' is strength update based sufficiently, counterfactually or Neccesarily robust w.r.t.
 w.r.t. to 'qbaf', a collection 'qbaf_collection', a subset of arguments 'explanation' and the inferences
 'inference_1' and 'inference_2'.
"""

def check_strength_update_based_SSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True



def check_strength_update_based_CSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True




def check_strength_update_based_NSI_explanations ( qbaf, argstar, epsilon, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_strength_update_collection ( qbaf, argstar, epsilon )

  for x in qbaf_collection :

    if explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) :
      return False

  return True

