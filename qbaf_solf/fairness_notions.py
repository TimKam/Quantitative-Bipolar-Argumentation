from qbaf import QBAFramework
from qbaf_solf.safety_oscillations_liveness import *
import math

def is_ideal_fair(qbaf_collection: list[QBAFramework],
                 topic_set: list[str],
                 threshold: float) -> bool:

    """
      Checks whether the qbaf_collection is ideally fair w.r.t. to qbaf_collection,
      topic_set and the threshold.
      
      Args:
        qbaf_collection (list[QBAFramework]): The QBAF dialogue.
        topic_set (list[str]): The topic set.
        threshold: The credibility threshold.
        
      Returns:
        bool: True if the qbaf_collection is ideally fair, false otherwise.
    """

    safe_check = [is_safe(qbaf_collection, [x], threshold) for x in topic_set]

    return True if (all(safe_check) or not any(safe_check)) else False


def is_live_fair(qbaf_collection: list[QBAFramework],
                 topic_set: list[str],
                 threshold: float) -> bool:
    """
          Checks whether the qbaf_collection is lively fair w.r.t. to qbaf_collection,
          topic_set and the threshold.
          
          Args:
            qbaf_collection (list[QBAFramework]): The QBAF dialogue.
            topic_set (list[str]): The topic set.
            threshold: The credibility threshold.
            
          Returns:
            bool: True if the qbaf_collection is lively fair, false otherwise.
    """

    live_check =[is_live(qbaf_collection, [x], threshold) for x in topic_set]

    return True if (all(live_check) or not any(live_check)) else False


def is_cautious_fair(qbaf_collection: list[QBAFramework],
                     topic_set: list[str],
                     threshold: float) -> bool:
    """
          Checks whether the qbaf_collection is cautiously fair w.r.t. to qbaf_collection,
          topic_set and the threshold.
          
          Args:
            qbaf_collection (list[QBAFramework]): The QBAF dialogue.
            topic_set (list[str]): The topic set.
            threshold: The credibility threshold.
            
          Returns:
            bool: True if the collection is cautiously fair, false otherwise.
    """

    safe_check =[is_safe(qbaf_collection, [x], threshold) for x in topic_set]
    live_check =[is_live(qbaf_collection, [x], threshold) for x in topic_set]

    return True if ((any(safe_check) and all(live_check)) or not any(safe_check)) else False

def calculate_area_under_curve(x_axis: list[float],
                               y_axis: list[float]) -> float:
    """
      Calculates the area under the curve defined by x_axis and the points in y_axis.
      
      Args: 
        x_axis (list[float]): The x_axis of the function.
        y_axis (list[float]): The y_axis of the function.
        
      Returns:
        float: returns the area under the curve.
    """

    area = sum([0.5 * (y_axis[i] + y_axis[i+1]) * (x_axis[i+1] - x_axis[i]) for i in range(0, len(x_axis)-1)])
    return area


def calculate_threshold_excess(qbaf_collection: list[QBAFramework],
                               topic_set: list[str],
                               threshold: float) -> int:

    """
      Calculates the threshold exceeding instances of the topic set.

      Args:
        qbaf_collection (list[QBAFramework]): The collection of QBAFs.
        topic_set (list[str]): The considered set of arguments.
        threshold (float): The credibility threshold.

      Returns:
        int: returns the number of threshold exceeding instances.
    """

    instances_of_credibility = dict()

    for topic_arg in topic_set:
      instances_of_credibility.update({topic_arg: len([qbaf for qbaf in qbaf_collection if (qbaf.final_strengths[topic_arg] >= threshold)])})

    return instances_of_credibility




def calculate_gini_fairness(qbaf_collection: list[QBAFramework],
                            topic_set: list[str],
                            threshold: float) -> list:

    """
      Calculates the Gini based fainress of the topic set.

      Args:
        qbaf_collection (list[QBAFramework]): The collection of QBAFs.
        topic_set (list[str]): The considered set of arguments.
        threshold (float): The credibility threshold.

      Returns:
        float: returns the Gini based fairness score.

    """

    x_axis = [x for x in range(0, len(topic_set)+1)]

    # Calculating the safety curve
    sorted_oscillations = sorted(calculate_threshold_excess(qbaf_collection, topic_set, threshold).items(), key=lambda item: item[1])
    safety_curve = [0]
    for x in sorted_oscillations:
      safety_curve.append(safety_curve[-1] + int(x[1]))

    # Calculating the fairness line
    fairness_line = [(safety_curve[-1]/len(topic_set)) * x for x in x_axis]

    # Calculating Gini fairness
    area_enclosed = abs(calculate_area_under_curve(x_axis, safety_curve) - calculate_area_under_curve(x_axis, fairness_line))
    gini_fairness = area_enclosed/(0.5 * (len(topic_set)-1) * fairness_line[-1])


    return gini_fairness


def calculate_shannon_fairness(qbaf_collection: list[QBAFramework],
                               topic_set: list[str],
                               threshold: float) -> list:

    """
      Calculates the Shannon based fainress of the topic set.

      Args:
        qbaf_collection (list[QBAFramework]): The collection of QBAFs.
        topic_set (list[str]): The considered set of arguments.
        threshold (float): The justification threshold.
        
      Returns:
        float: returns the Shannon based fairness score.

    """

    if (len(topic_set) <= 1):
        return 1

    
    x_axis = [x for x in range(0, len(topic_set)+1)]


    # Calculating the oscillation probability
    oscillations = calculate_threshold_excess(qbaf_collection, topic_set, threshold)
    if all(oscillations[x] == 0 for x in oscillations.keys()):
      return 1
    sum_of_oscillations = sum([x[1] for x in oscillations.items()])
    oscillation_probability = {x: oscillations[x]/sum_of_oscillations for x in oscillations.keys()}

    # Calculating the information coefficient
    info_coefficient = {x: (-math.log(oscillation_probability[x])/math.log(len(topic_set))) for x in oscillations.keys() if oscillation_probability[x] > 0}
    shannon_fairness = sum([oscillation_probability[x] * info_coefficient[x] for x in info_coefficient.keys()])


    return shannon_fairness



