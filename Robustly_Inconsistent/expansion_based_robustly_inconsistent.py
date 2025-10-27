from qbaf import QBAFramework, QBAFARelations
from Robust_Inferences.expansion_based_robust_inferences import *
from Robust_Inferences.general_robust_inferences import check_general_robustly_inconsistent


"""
 Function to check whether a QBAF is robustly inconsistence over the collection
 of qbafs 'qbaf_collection' made of expansions, i.e. it checks whether there is always a change of
 inference between qbaf and x in 'qbaf_collection', which consists of sub-QBAFs between qbaf_inital
 and qbaf_final.
"""

def check_expansion_based_robustly_inconsistent ( qbaf_initial, qbaf_final, inference_1, inference_2 ) :

  G = generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final )

  return check_general_robustly_inconsistent ( qbaf_initial, G, inference_1, inference_2 )