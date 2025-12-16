import pytest
from qbaf import QBAFramework
from robustness_checks.robust_consistency_checks import *
from robustness_checks.robust_inconsistency_checks import *

from robustness_checks.pocket_and_explanations import *

args_1 = ['a', 'b', 'c']
args_2 = ['a', 'b', 'c', 'd']
args_3 = ['a', 'b', 'c', 'd', 'e']
args_4 = ['a', 'b', 'c', 'e']

att_1 = [('c', 'a')]
att_2 = [('c', 'a'), ('d', 'c')]
att_3 = [('c', 'a'), ('d', 'c'),]
att_4 = [('c', 'a'),]

supp_1 = [('c', 'b')]
supp_2 = [('c', 'b')]
supp_3 = [('c', 'b'), ('e', 'c')]
supp_4 = [('c', 'b'), ('e', 'c')]

strength_1 =[1.0, 2.0, 1.0]
strength_2 =[1.0, 2.0, 1.0, 1.0]
strength_3 =[1.0, 2.0, 1.0, 0.5, 1.5]
strength_4 =[1.0, 2.0, 1.0, 0.5]

qbaf_initial = QBAFramework(args_1, strength_1, att_1, supp_1, semantics = "QuadraticEnergy_model")

qbaf_u_1 = QBAFramework(args_2, strength_2, att_2, supp_2, semantics = "QuadraticEnergy_model")

qbaf_u_2 = QBAFramework(args_3, strength_3, att_3, supp_3, semantics = "QuadraticEnergy_model")

qbaf_u_3 = QBAFramework(args_4, strength_4, att_4, supp_4, semantics = "QuadraticEnergy_model")



def test_robust_consistency():
  value_1 = general_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
  
  value_2 = general_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
  assert value_1 == True
  assert value_2 == True




def test_expansion_robust_consistent():
   value_1 = expansion_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')

   value_2 = expansion_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
   assert value_1 == True
   assert value_2 == True



def test_robust_inconsistency():
  value_1 = general_robust_inconsistent(qbaf_initial,
                                        qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                        topic_argument_1 = 'a',
                                        topic_argument_2 = 'b')
  
  value_2 = general_robust_inconsistent(qbaf_initial,
                                        qbaf_collection = [qbaf_initial],
                                        topic_argument_1 = 'a',
                                        topic_argument_2 = 'b')
    
  assert value_1 == False
  assert value_2 == False


def test_expnasion_robust_inconsistency():
   value_1 = expansion_robust_inconsistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')

   value_2 = expansion_robust_inconsistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
   assert value_1 == True
   assert value_2 == True


def test_pockets():

  value_1 = pockets_of_consistency(qbaf_initial,
                                     qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b') 
    
  value_2 = pockets_of_consistency(qbaf_initial,
                                     qbaf_collection = [qbaf_initial],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
    
  v_1 = [set(), {'d'}, {'e'}, {'d', 'e'}]

  for x in value_1: assert x in v_1
  for x in v_1: assert x in value_1

  assert value_2 == [set()]
    


def test_explanations():
  value_1 = explanations_of_updates(qbaf_initial,
                                     qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b') 
    
  value_2 = explanations_of_updates(qbaf_initial,
                                     qbaf_collection = [qbaf_initial],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
  
  v_1 = [(set(), 'd'), (set(), 'e'), ({'d'}, 'e'), ({'e'}, 'd')]
  
  for x in v_1: assert  x in value_1
  for x in value_1: assert x in v_1

  assert value_2 == []
  
  