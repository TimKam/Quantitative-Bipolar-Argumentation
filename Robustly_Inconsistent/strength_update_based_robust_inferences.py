from qbaf import QBAFramework, QBAFARelations
from Robust_Inferences.strength_update_based_robust_inferences import *
from Robustly_Inconsistent.general_robustly_inconsistent import check_general_robustly_inconsistent


"""
 Function to check whether a QBAF is robustly inconsistence over the collection
 of qbafs 'qbaf_collection' of strength updates, i.e. it checks whether there is always a change of
 inference between qbaf and x in 'qbaf_collection' which a collection of strength updates over 'qbaf'
 and 'argstar'.
"""

def check_update_based_robustly_inconsistent ( qbaf, argstar, epsilon, inference_1, inference_2 ) :

  G = generate_strength_update_collection ( qbaf, argstar, epsilon )

  return check_general_robustly_inconsistent ( qbaf, G, inference_1, inference_2 )

