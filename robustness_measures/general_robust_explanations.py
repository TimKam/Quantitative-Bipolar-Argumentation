from qbaf import QBAFramework

def check_robust_SSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):
  """checks whether an explanation is generally sufficiently robust w.r.t. 
     qbaf and qbaf_collection. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): The collection of QBAF updates against which gen. robust sufficieny will be tested.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """


  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalSSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True




def check_robust_CSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):
  """checks whether an explanation is generally counterfactually robust w.r.t. 
     qbaf and qbaf_collection. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): The collection of QBAF updates against which gen. robust counter-factuality will be tested.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalCSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True



def check_robust_NSI_explanations_general ( explanation, qbaf, qbaf_collection, inference_1, inference_2 ):
  """checks whether an explanation is generally necessarily robust w.r.t. 
     qbaf and qbaf_collection. 

  Args:
      qbaf (QBAFramewrok): The initial QBAF.
      qbaf_collection (list): The collection of QBAF updates against which gen. robust necessity will be tested.
      explanation (dictionary): A subset of arguments.
      inference_1 (string): The first inference.
      influence_2 (string): The second inference. 

  Returns:


      bool: The final truth value of the above described check.
  """

  for x in qbaf_collection:

    if ( explanation not in qbaf.minimalNSIExplanations ( x, inference_1, inference_2 ) ) :
      return False

  return True
