from qbaf import QBAFramework, QBAFARelations
from robustness_measures.expansion_based_robust_inferences import *
from robustness_measures.general_robustly_inconsistent import check_general_robustly_inconsistent




def check_expansion_based_robustly_inconsistent ( qbaf_initial, qbaf_final, inference_1, inference_2 ) :
  """checks whether a QBAF is robustly inconsistent over the collection,
     made of expansions between qbaf_initila and qbaf_final.

  Args:
      qbaf_initial (QBAFramewrok): The initial QBAF.
      qbaf_final (QBAFramewrok): The updated QBAF.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:
      bool: The final truth value of the above described check.
  """

  G = generate_expansion_based_qbaf_collection ( qbaf_initial, qbaf_final )

  return check_general_robustly_inconsistent ( qbaf_initial, G, inference_1, inference_2 )