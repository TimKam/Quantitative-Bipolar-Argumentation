import random

from qbaf import QBAFramework

"""Utils"""


def _QBAF_relations(qbaf:QBAFramework) -> list:
  """Get all support and attack relationships of a QBAF.

  Args:
      qbaf (QBAFramework): QBAF of interest

  Returns:
      list: List of all support and attack relationships
  """
  return list(qbaf.attack_relations.relations) + list(qbaf.support_relations.relations)


def _QBAF_contains_relation(qbaf:QBAFramework, agent, patient) -> bool:
  """Check if a given QBAF contains a specific relationship (either support or attack).

  Args:
      qbaf (QBAFramework): QBAF of interest
      agent (_type_): Source argument
      patient (_type_): Target argument

  Returns:
      bool: True if the relationship exists; else false.
  """
  return qbaf.contains_attack_relation(agent, patient) or qbaf.contains_support_relation(agent, patient)


def _pop_random(l:list):
  """Pops a random element from a list.

  Args:
      l (list): List of interest

  Returns:
      any: Popped list item
  """
  assert len(l) > 0
  index = random.randint(0, len(l) - 1)
  item = l[index]
  del l[index]
  return item


def _get_random(l:list):
  """Gets a random element of a list.

  Args:
      l (list): List of interest

  Returns:
      any: Retrieved list item
  """
  assert len(l) > 0
  index = random.randint(0, len(l) - 1)
  item = l[index]
  return item

"""End: utils"""


def modify_QBAF_random(qbaf:QBAFramework, n_modify_arguments:int, n_remove_relations:int, n_add_relations:int) -> QBAFramework:
  """Modifies a QBAF randomly.
  Does not include removal and addition of arguments.

  Args:
      qbaf (QBAFramework): QBAF of interest
      n_modify_arguments (int): Number of arguments whose strengths should be changed
      n_remove_relations (int): Number of relationships that should be removed
      n_add_relations (int): Number of relationships that should be added

  Returns:
      QBAFramework: Modified QBAF
  """
  assert n_modify_arguments >= 0 and n_modify_arguments <= len(qbaf.arguments)
  assert n_remove_relations >= 0 and n_remove_relations <= (len(qbaf.attack_relations) + len(qbaf.support_relations))
  assert n_add_relations >= 0

  # Modify arguments
  arguments = list(qbaf.arguments)
  for _ in range(n_modify_arguments):
    arg = _pop_random(arguments)
    i_strength = random.uniform(0, 1)
    qbaf.modify_initial_strength(arg, i_strength)

  # Remove relations
  relations = _QBAF_relations(qbaf)
  for _ in range(n_remove_relations):
    agent, patient = _pop_random(relations)
    qbaf.remove_attack_relation(agent, patient)
    qbaf.remove_support_relation(agent, patient)

  # Add relations
  added_relations = 0
  while added_relations < n_add_relations:
    arguments = list(qbaf.arguments)
    arg1 = _pop_random(arguments)
    arg2 = _get_random(arguments)
    if _QBAF_contains_relation(qbaf, arg1, arg2):
      continue
    if random.randint(0, 1) == 1:
      qbaf.add_attack_relation(arg1, arg2)
    else:
      qbaf.add_support_relation(arg1, arg2)
    added_relations += 1

    # Check that the QBAF is acyclic
    if not qbaf.isacyclic():
      qbaf.remove_attack_relation(arg1, arg2)
      qbaf.remove_support_relation(arg1, arg2)
      added_relations -= 1

  return qbaf


def modify_QBAF_random_update(qbaf:QBAFramework, n_modify_arguments:int) -> QBAFramework:
  """Modifies a QBAF randomly, for generating change explanations.
  Includes removal and addition of arguments.

  Args:
      qbaf (QBAFramework): QBAF of interest.
      n_modify_arguments (int): Number of arguments that should be modified.

  Returns:
      QBAFramework: Modified QBAF.
  """
  assert n_modify_arguments >= 0 and n_modify_arguments <= len(qbaf.arguments)

  qbaf_backup = qbaf.copy()
  # Modify arguments
  arguments = list(qbaf.arguments)
  for i in range(n_modify_arguments):
    if i <= i/5: # remove argument
      arg = _pop_random(arguments)
      relations = _QBAF_relations(qbaf)
      for agent, patient in relations:
        if agent == arg or patient == arg:
          qbaf.remove_attack_relation(agent, patient)
          qbaf.remove_support_relation(agent, patient)
      qbaf.remove_argument(arg)
    elif i <= 2 * (i/5): # change strength
      arg = _pop_random(arguments)
      i_strength = random.uniform(0, 1)
      qbaf.modify_initial_strength(arg, i_strength)
    elif i <= 3 * (i/5):
      # remove relation
      relations = _QBAF_relations(qbaf)
      agent, patient = _pop_random(relations)
      qbaf.remove_attack_relation(agent, patient)
      qbaf.remove_support_relation(agent, patient)
    elif i <= 3 * (i/5): # add relation
      arguments = list(qbaf.arguments)
      arg1 = _pop_random(arguments)
      arg2 = _get_random(arguments)
      if _QBAF_contains_relation(qbaf, arg1, arg2):
        continue
      if random.randint(0, 1) == 1:
        qbaf.add_attack_relation(arg1, arg2)
      else:
        qbaf.add_support_relation(arg1, arg2)
    else: # add argument
        qbaf.add_argument(f'a{i}', random.uniform(0, 1))
        arguments = list(qbaf.arguments)
        # add on average 1 relation, outgoing
        for _ in range(random.choice([0, 1, 2])):
          relations = _QBAF_relations(qbaf)
          arg2 = _pop_random(arguments)
          if random.randint(0, 1) == 1:
            qbaf.add_attack_relation(f'a{i}', arg2)
          else:
            qbaf.add_support_relation(f'a{i}', arg2)
        # add on average 1/5 relation, incoming
        for _ in range(random.choice([0, 0, 0, 0, 1])):
          relations = _QBAF_relations(qbaf)
          arg2 = _pop_random(arguments)
          if random.randint(0,1) == 1:
            qbaf.add_attack_relation(arg2, f'a{i}')
          else:
            qbaf.add_support_relation(arg2, f'a{i}')

  # Check that the QBAF is acyclic
  if not qbaf.isacyclic():
    return modify_QBAF_random_update(qbaf_backup, n_modify_arguments)

  return qbaf


def QBAF_random(n_arguments:int, n_relations:int, semantics="QuadraticEnergy_model") -> QBAFramework:
    """Create a random QBAF with the specified number of arguments and relationships.

    Args:
        n_arguments (int): Number of arguments
        n_relations (int): Number of relationships
        semantics (str, optional): Gradual semantics. Defaults to "QuadraticEnergy_model".

    Returns:
        QBAFramework: QBAF
    """
    assert n_arguments >= 0 and n_relations >= 0
    # Create arguments as strings of the numbers 1..n_arguments
    args = [str(n+1) for n in range(n_arguments)]
    # Generate random strengths in range [0,1] for each argument
    i_strengths = [random.uniform(0, 1) for _ in range(n_arguments)]
    # Create the QBAF without relations
    qbaf = QBAFramework(args, i_strengths, [], [], semantics=semantics)
    # Add the relations
    modify_QBAF_random(qbaf, n_modify_arguments=0, n_remove_relations=0, n_add_relations=n_relations)
    # Return the QBAF
    return qbaf


def random_inconsistent_arguments(qbf1:QBAFramework, qbf2:QBAFramework) -> tuple:
    """Retrieves two random arguments that are strength inconsistent across two QBAFs.

    Args:
        qbf1 (QBAFramework): First QBAF
        qbf2 (QBAFramework): Second QBAF

    Returns:
        tuple: Tuple of strength-inconsistent arguments
    """
    arguments = list(qbf1.arguments.intersection(qbf2.arguments))
    s_inconstent_args = [(arg1, arg2) for i, arg1 in enumerate(arguments) for arg2 in arguments[i:] if not qbf1.are_strength_consistent(qbf2, arg1, arg2)]
    if len(s_inconstent_args) > 0:
        return _get_random(s_inconstent_args)
    return None


def random_QBAFs(n_arguments:int,
                 prop_relations=3,
                 semantics="QuadraticEnergy_model",
                 size=1,
                 seed=1234
                 ) -> list:
  """Generates a list of random QBAFs

  Args:
      n_arguments (int): Number of arguments in each QBAF.
      prop_relations (int, optional): Number of relations in proportion to `n_arguments` (generated QBAF). Defaults to 3.
      semantics (str, optional): Gradual semantics. Defaults to "QuadraticEnergy_model".
      size (int, optional): Number of generated QBAFs. Defaults to 1.

  Returns:
      list: _description_
  """
  if seed is not None:
    random.seed(seed)

  n_relations = int(n_arguments * prop_relations)

  result = []
  while len(result) < size:
    qbaf = QBAF_random(n_arguments, n_relations, semantics=semantics) # generated QBAF
    result.append(qbaf)  # generated_QBAF

  return result


def generate_random_QBAFs(n_arguments:int,
                          prop_relations=1.1,
                          prop_modify_arguments=0.2,
                          semantics="QuadraticEnergy_model",
                          size=1,
                          seed=1234
                          ) -> list:
  """Generates a list of tuples of random QBAFs and their updates

  Args:
      n_arguments (int): Number of arguments (generated QBAF)
      prop_relations (number, optional): Number of relations in proportion to `n_arguments` (generated QBAF). Defaults to 1.1.
      prop_modify_arguments (number, optional):  Number of modified arguments in proportion to n_arguments (updated QBAF). Defaults to 0.2.
      size (int, optional): Number of pairs generated. Defaults to 1.
      seed (int, optional): Random seed. Defaults to 1234.

  Returns:
      list: List of QBAF tuples
  """
  if seed is not None:
    random.seed(seed)

  n_relations = int(n_arguments * prop_relations)
  n_modify_arguments = int(n_arguments * prop_modify_arguments)

  result = []
  while len(result) < size:
    qbaf1 = QBAF_random(n_arguments, n_relations, semantics=semantics) # generated QBAF
    qbaf2 = qbaf1.copy()
    qbaf2 = modify_QBAF_random_update(qbaf2, n_modify_arguments)
    s_inconsistent_args = random_inconsistent_arguments(qbaf1, qbaf2)
    if s_inconsistent_args is None:
      continue
    result.append((qbaf1, qbaf2, s_inconsistent_args))  # generated_QBAF, updated_QBAF, (arg1, arg2)

  return result # Return a list of tuples (generated_QBAF:QBAFramework, updated_QBAF:QBAFramework, pair_strength_inconsistent_arguments:tuple)