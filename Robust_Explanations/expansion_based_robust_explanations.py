from qbaf import QBAFramework, QBAFARelations
from Robust_Inferences.expansion_based_robust_inferences import *
from Robust_Explanations.general_robust_explanations import *


"""
 Function that checks whether 'explnation' is expansion based Sufficiently, Countgerfactually or Necessarily robust w.r.t.
 w.r.t. to 'qbaf', a collection 'qbaf_collection', a subset of arguments 'explanation' and the inferences
 'inference_1' and 'inference_2'.
"""


def check_expansion_based_SSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )




def check_expansion_based_CSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )



def check_expansion_based_NSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )
