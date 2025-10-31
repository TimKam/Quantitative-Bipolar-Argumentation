# Check Robustness w.r.t. arbitrary collection of updates

from qbaf import QBAFramework 

def check_general_robust_inferences ( qbaf_0, QBAFUpdates, a, b ):

  for item in QBAFUpdates:

    if ( qbaf_0. are_strength_consistent( item, a, b ) == False ):
      return False

  return True


