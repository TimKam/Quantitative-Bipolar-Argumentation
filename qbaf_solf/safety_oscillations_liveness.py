from qbaf import QBAFramework

def is_safe(qbaf_collection: list[QBAFramework],
            topic_set: list[str],
            threshold: float) -> bool:
    """
      Returns whether a topic set is safe w.r.t. qbaf_collection, and treshold.

      Args:
        qbaf_collection (list[QBAFramework]): The list QBAFs w.r.t. which safety is measured.
        topic_set (list[str]): The set of topic arguments for which safety is measured.
        threshold (float): The threshold of the topic set.

      Returns:
        bool: true if the topic_set is safe, false otherwise.

    """

    for qbaf in qbaf_collection:
      for arg in topic_set:
        if (qbaf.final_strength(arg)<threshold):
          return False

    return True


def is_live(qbaf_collection: list[QBAFramework],
            topic_set: list[str],
            threshold: float) -> bool:
    """
      Returns whether a topic set is live w.r.t. qbaf_collection, and treshold.

      Args:
        qbaf_collection (list[QBAFramework]): The list QBAFs w.r.t. which liveness is measured.
        topic_set (list[str]): The set of topic arguments for which liveness is measured.
        threshold (float): The threshold of the topic set.

      Returns:
        bool: true if the topic_set is live, false otherwise.

    """

    qbaf = qbaf_collection[len(qbaf_collection)-1]
    for arg in topic_set:
      if (qbaf.final_strength(arg)<threshold):
        return False

    return True


def number_of_oscillations(qbaf_collection: list[QBAFramework],
                           topic_set: list[str],
                           threshold: float) -> dict:

    """
      Returns the number of oscillations of a topic set w.r.t. qbaf_collection, and treshold.

      Args:
        qbaf_collection (list[QBAFramework]): The list QBAFs.
        topic_set (list[str]): The set of topic arguments for which the oscillations are measured.
        threshold (float): The threshold of the topic set.

      Returns:
        dict: the number of oscillations of the topic_set.

    """
    
    oscillations = {x: 0 for x in topic_set}

    for i in range(0, len(qbaf_collection)-1):
      for x in topic_set:
        if (qbaf_collection[i].final_strength(x)< threshold and qbaf_collection[i+1].final_strength(x) >= threshold):
          oscillations[x] += 1
        elif (qbaf_collection[i].final_strength(x)>= threshold and qbaf_collection[i+1].final_strength(x) < threshold):
          oscillations[x] += 1


    return oscillations
