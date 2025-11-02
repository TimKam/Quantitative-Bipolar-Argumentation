from qbaf import QBAFramework
from robustness_measures.expansion_based_robust_inferences import *
from robustness_measures.general_robust_explanations import *


def check_expansion_based_SSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is expansion based sufficiently robust w.r.t. 
     qbaf_initial and qbaf_final. The QBAF updates considered is the collection of
     sub-QBAFs between qbaf_initial and qbaf_final. 

  Args:
      qbaf_initial (QBAFramewrok): The initial QBAF.
      qbaf_final (QBAFramework): The updated QBAF w.r.t. which the QBAF updates are generated .
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )




def check_expansion_based_CSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is expansion based counterfactually robust w.r.t. 
     qbaf_initial and qbaf_final. The QBAF updates considered is the collection of
     sub-QBAFs between qbaf_initial and qbaf_final. 

  Args:
      qbaf_initial (QBAFramewrok): The initial QBAF.
      qbaf_final (QBAFramework): The updated QBAF w.r.t. which the QBAF updates are generated .
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )



def check_expansion_based_NSI_explanations ( qbaf_initial, qbaf_final, explanation, inference_1, inference_2 ) :
  """checks whether an explanation is expansion based necessarily robust w.r.t. 
     qbaf_initial and qbaf_final. The QBAF updates considered is the collection of
     sub-QBAFs between qbaf_initial and qbaf_final. 

  Args:
      qbaf_initial (QBAFramewrok): The initial QBAF.
      qbaf_final (QBAFramework): The updated QBAF w.r.t. which the QBAF updates are generated.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  qbaf_collection =  generate_expansion_based_qbaf_collection( qbaf_initial, qbaf_final )

  return check_robust_SSI_explanations_general ( explanation, qbaf_initial, qbaf_collection, inference_1, inference_2 )
