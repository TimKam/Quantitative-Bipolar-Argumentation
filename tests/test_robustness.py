from qbaf import QBAFramework
import copy
from qbaf_robustness.consistency_checks import *
from qbaf_robustness.inconsistency_checks import *
from qbaf_robustness.explanations import *

args_1 = ['a', 'b', 'c']
args_2 = ['a', 'b', 'c', 'd']
args_3 = ['a', 'b', 'c', 'd', 'e']
args_4 = ['a', 'b', 'c', 'e']
args_5 = copy.deepcopy(args_4)
args_5.append('f')

att_1 = [('c', 'a')]
att_2 = [('c', 'a'), ('d', 'c')]
att_3 = [('c', 'a'), ('d', 'c'),]
att_4 = [('c', 'a')]
att_5 = [('c', 'a'), ('f', 'a')]

supp_1 = [('c', 'b')]
supp_2 = [('c', 'b')]
supp_3 = [('c', 'b'), ('e', 'c')]
supp_4 = [('c', 'b'), ('e', 'c')]

strength_1 =[1.0, 2.0, 1.0]
strength_2 =[1.0, 2.0, 1.0, 1.0]
strength_3 =[1.0, 2.0, 1.0, 0.5, 1.5]
strength_4 =[1.0, 2.0, 1.0, 0.5]
strength_5 =[10.0, 2.0, 1.0, 0.5]
strength_6 =[10.0, 2.0, 1.0, 0.5, 5.0]

qbaf_initial = QBAFramework(args_1, strength_1, att_1, supp_1, semantics = "QuadraticEnergy_model")

qbaf_u_1 = QBAFramework(args_2, strength_2, att_2, supp_2, semantics = "QuadraticEnergy_model")

qbaf_u_2 = QBAFramework(args_3, strength_3, att_3, supp_3, semantics = "QuadraticEnergy_model")

qbaf_u_3 = QBAFramework(args_4, strength_4, att_4, supp_4, semantics = "QuadraticEnergy_model")

qbaf_u_4 = QBAFramework(args_4, strength_5, att_4, supp_4, semantics = "QuadraticEnergy_model")

qbaf_u_5 = QBAFramework(args_5, strength_6, att_5, supp_4, semantics = "QuadraticEnergy_model")

Q_2 = QBAFramework(['a', 'b', 'c', 'd'], [0.5, 0.5, 0.5, 0.5], [('c', 'a')], [('d', 'b')], semantics = "QuadraticEnergy_model")

Q_1 = QBAFramework(['a', 'b'], [0.5, 0.5], [], [], semantics="QuadraticEnergy_model")

Q_3 = QBAFramework(['a', 'b', 'c', 'd', 'e'], [0.5, 0.5, 0.5, 0.5, 0.0], [('c', 'a')], [('d', 'b')], semantics = "QuadraticEnergy_model")

value = pockets_of_consistency(qbaf_initial,
                               qbaf_collection = [qbaf_u_4, qbaf_u_5],
                               topic_argument_1 = 'a',
                               topic_argument_2 = 'b')
exps = explanation_of_robustness_violation(qbaf_initial,
                               qbaf_collection = [qbaf_u_4, qbaf_u_5],
                               topic_argument_1 = 'a',
                               topic_argument_2 = 'b')



def test_robust_consistency():
  generated_value_1 = is_general_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
  
  generated_value_2 = is_general_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
  assert generated_value_1 == True
  assert generated_value_2 == True




def test_expansion_robust_consistency():
   generated_value_1 = is_expansion_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')

   generated_value_2 = is_expansion_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
   assert generated_value_1 == True
   assert generated_value_2 == True




def test_robust_inconsistency():
  generated_value_1 = is_general_robust_inconsistent(qbaf_initial,
                                        qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                        topic_argument_1 = 'a',
                                        topic_argument_2 = 'b')
  
  generated_value_2 = is_general_robust_inconsistent(qbaf_initial,
                                        qbaf_collection = [qbaf_initial],
                                        topic_argument_1 = 'a',
                                        topic_argument_2 = 'b')
    
  assert generated_value_1 == False
  assert generated_value_2 == False




def test_expnasion_robust_inconsistency():
   generated_value_1 = is_expansion_robust_inconsistent(qbaf_initial,
                                      qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')

   generated_value_2 = is_expansion_robust_consistent(qbaf_initial,
                                      qbaf_collection = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b')
   assert generated_value_1 == True
   assert generated_value_2 == True




def test_bounded_updates_robust_consistency():
   generated_value_1 = is_bounded_updates_robust_consistent(qbaf_initial,
                                      qbaf_updates = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b',
                                      epsilon = 0.01, 
                                      mutable_args = {'d'})

   generated_value_2 = is_bounded_updates_robust_consistent(qbaf_initial,
                                      qbaf_updates = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b',
                                      epsilon = 0.01, 
                                      mutable_args = {'a'})
   assert generated_value_1 == True
   assert generated_value_2 == True




def test_bounded_updates_robust_inconsistency():
   generated_value_1 = is_bounded_updates_robust_inconsistent(qbaf_initial,
                                      qbaf_updates = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b',
                                      epsilon = 0.01, 
                                      mutable_args = {'d'})

   generated_value_2 = is_bounded_updates_robust_inconsistent(qbaf_initial,
                                      qbaf_updates = [qbaf_initial],
                                      topic_argument_1 = 'a',
                                      topic_argument_2 = 'b',
                                      epsilon = 0.01, 
                                      mutable_args = {'a'})
   assert generated_value_1 == True
   assert generated_value_2 == False




def test_pockets():
  generated_value_1 = pockets_of_consistency(qbaf_initial,
                                     qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b') 
    
  generated_value_2 = pockets_of_consistency(qbaf_initial,
                                     qbaf_collection = [qbaf_initial],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
  generated_value_3 = pockets_of_consistency(qbaf_initial,
                               qbaf_collection = [qbaf_u_4, qbaf_u_5],
                               topic_argument_1 = 'a',
                               topic_argument_2 = 'b')
    
  expected_value_1 = [{'d', 'e'}]
  expected_value_3 = [{'f'}]

  for x in generated_value_1: assert x in expected_value_1
  for x in expected_value_1: assert x in generated_value_1

  for x in generated_value_3: assert x in expected_value_3
  for x in expected_value_3: assert x in generated_value_3

  assert generated_value_2 == []




def test_explanations():
  generated_value_1 = explanation_of_robustness_violation(qbaf_initial,
                                     qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b') 
    
  generated_value_2 = explanation_of_robustness_violation(qbaf_initial,
                                     qbaf_collection = [qbaf_initial],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')

  generated_value_3 = explanation_of_robustness_violation(qbaf_initial,
                               qbaf_collection = [qbaf_u_4, qbaf_u_5],
                               topic_argument_1 = 'a',
                               topic_argument_2 = 'b')
  
  expected_value_1 = [({'d','e'}, set())]
  expected_value_3 = [({'f'},'e')]
  
  for x in expected_value_1: assert  x in generated_value_1
  for x in generated_value_1: assert x in expected_value_1

  for x in generated_value_3: assert x in expected_value_3
  for x in expected_value_3: assert x in generated_value_3

  assert generated_value_2 == []


def test_max_pockets():
   g_value_1 = determine_max_pockets(qbaf_initial,
                                     qbaf_collection = [qbaf_u_1, qbaf_u_2, qbaf_u_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b') 
    
   generated_value_2 = determine_max_pockets(qbaf_initial,
                                     qbaf_collection = [qbaf_initial],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
   
   g_value_3 = determine_max_pockets(qbaf_initial,
                               qbaf_collection = [qbaf_u_4, qbaf_u_5],
                               topic_argument_1 = 'a',
                               topic_argument_2 = 'b')
   
   g_value_4 = determine_max_pockets(qbaf_initial= Q_1, 
                                     qbaf_collection = [Q_2],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
    
   generated_value_1 = [set(x) for x in g_value_1]
   generated_value_3 = [set(x) for x in g_value_3] 
   generated_value_4 = [set(x) for x in g_value_4]
   expected_value_1 = [{'d', 'e'}]
   expected_value_3 = [{'f'}]
   expected_value_4 = [{'c'}, {'d'}]

   for x in generated_value_1: assert x in expected_value_1
   for x in expected_value_1: assert x in generated_value_1

   for x in generated_value_3: assert x in expected_value_3
   for x in expected_value_3: assert x in generated_value_3

   for x in generated_value_4: assert x in expected_value_4
   for x in expected_value_4: assert x in generated_value_4

   assert generated_value_2 == [[]]

  
def test_max_linear_pockets():

   g_value_1 = determine_linear_pragmatic_pockets(qbaf_initial= Q_1, 
                                     qbaf_collection = [Q_2],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
   
   g_value_2 = determine_linear_pragmatic_pockets(qbaf_initial= Q_1, 
                                     qbaf_collection = [Q_3],
                                     topic_argument_1 = 'a',
                                     topic_argument_2 = 'b')
   
   generated_value_1 = [set(x) for x in g_value_1]
   generated_value_2 = [set(x) for x in g_value_2]

   expected_value_1 = [{'c'}, {'d'}]
   expected_value_2 = [{'c', 'd'}, {'c', 'e'}, {'d', 'e'}]

   for x in generated_value_1: assert x in expected_value_1
   for x in expected_value_1: assert x in generated_value_1

   for x in generated_value_2: assert x in expected_value_2
   for x in expected_value_2: assert x in generated_value_2
