from qbaf import QBAFramework
from qbaf_robustness.explanations import *
from qbaf_robustness.consistency_checks import *
from qbaf_robustness.inconsistency_checks import *


args = ['a', 'b', 'c', 'd', 'e', 'f']
initial_strengths = [0.5, 0.2, 0.85, 0.65, 0.4, 1]
atts = [('c', 'a'), ('e', 'b'), ('f', 'c')]
supps = [('b', 'a'), ('d', 'a'), ('e', 'c')]

args_1 = ['a', 'b', 'c', 'd', 'e', 'g', 'h']
initial_strengths_1 = [0.5, 0.2, 0.77, 0.5, 0.994, 1, 0.04]
atts_1 = [('c', 'a'), ('e', 'b'), ('g', 'c')]
supps_1 = [('b', 'a'), ('d', 'a'), ('h', 'c')]

args_2 = ['a', 'b', 'c', 'd']
initial_strengths_2 = [0.5, 0.2, 0.77, 0.3]
atts_2 = [('c', 'a'), ('d', 'b')]
supps_2 = [('b', 'a'), ('d', 'a'), ('d', 'c')]


args_3 = ['a', 'b']
initial_strengths_3 = [0.1, 0.4]

args_4 = ['a', 'b', 'c', 'd' ]
initial_strengths_4 =[0.2, 0.5, 0.7, 0.1]
atts_4 = [('c', 'b')]
supps_4 = [('c', 'a')]


QBAF_0 = QBAFramework ( args, initial_strengths, atts, supps, semantics="QuadraticEnergy_model" )
QBAF_1 = QBAFramework ( args_1, initial_strengths_1, atts_1, supps_1, semantics="QuadraticEnergy_model" )
QBAF_2 = QBAFramework ( args_2, initial_strengths_2, atts_2, supps_2, semantics="QuadraticEnergy_model" )
QBAF_3 = QBAFramework ( args_3, initial_strengths_3, [], [], semantics="QuadraticEnergy_model" )
QBAF_4 = QBAFramework ( args_3, [ 0.2, 0.2 ], [], [], semantics="QuadraticEnergy_model" )
QBAF_5 = QBAFramework ( args_4, initial_strengths_4, atts_4, supps_4, semantics="QuadraticEnergy_model" )

def test_robust_SSI_explanations_general ():

   assert ( check_robust_SSI_explanations_general ( {'a'} , QBAF_0, [ QBAF_3 ], 'a', 'b') == True )
   

def test_robust_CSI_explanations_general () :

    assert ( check_robust_SSI_explanations_general ( {'a'} , QBAF_0, [ QBAF_3 ], 'a', 'b') == True )
    assert ( check_robust_CSI_explanations_general ( {'a', 'f'} , QBAF_0, [ QBAF_3 ], 'a', 'b') == False )

def test_robust_NSI_explanations_general ():

    assert ( check_robust_NSI_explanations_general ( {'a'} , QBAF_0, [ QBAF_3 ], 'a', 'b') == False )

def test_expansion_based_SSI_explanations ():
    
    assert ( check_expansion_based_SSI_explanations ( QBAF_3, QBAF_2, {'a'}, 'a', 'b') == False)

def test_expansion_based_CSI_explanations ():
  
    assert ( check_expansion_based_CSI_explanations ( QBAF_3, QBAF_2, {'a'}, 'a', 'b') == False)

def test_expansion_based_NSI_explanations ():
    
    assert ( check_expansion_based_NSI_explanations ( QBAF_3, QBAF_2, {'a'}, 'a', 'b') == False)

def test_strength_update_based_SSI_explanations ():

    assert ( check_strength_update_based_SSI_explanations (QBAF_4, [ 'a' ], 0.4, {'a'}, 'a', 'b' ) == False)

def test_strength_update_based_CSI_explanations ():

    assert ( check_strength_update_based_CSI_explanations (QBAF_4, [ 'a' ], 0.4, {'a'}, 'a', 'b' ) == False)

def test_strength_update_based_NSI_explanations ():

    assert ( check_strength_update_based_NSI_explanations (QBAF_4, [ 'a' ], 0.4, {'a'}, 'a', 'b' ) == False)

def test_ctrb_based_SSI_explanations_removal ():
    
    assert ( check_ctrb_based_robust_SSI_explanations ( QBAF_3, {'d'}, 1, [QBAF_5], 'a', 'b') == True )

def test_ctrb_based_CSI_explanations_removal ():
    
    assert ( check_ctrb_based_robust_CSI_explanations ( QBAF_3, {'d'}, 1, [QBAF_5], 'a', 'b') == True )

def test_ctrb_based_NSI_explanations_removal ():
    
    assert ( check_ctrb_based_robust_NSI_explanations ( QBAF_3, {'d'}, 1, [QBAF_5], 'a', 'b') == True )