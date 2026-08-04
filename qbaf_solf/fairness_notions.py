from qbaf import QBAFramework
from qbaf_solf.safety_oscillations_liveness import *
import numpy as np
import math

def is_ideal_fair(qbaf_collection: list[QBAFramework],
                 topic_set: list[str],
                 threshold: float) -> bool:

    safe_check = [is_safe(qbaf_collection, [x], threshold) for x in topic_set]

    return True if (all(safe_check) or not any(safe_check)) else False


def is_live_fair(qbaf_collection: list[QBAFramework],
                 topic_set: list[str],
                 threshold: float) -> bool:

    live_check =[is_live(qbaf_collection, [x], threshold) for x in topic_set]

    return True if (all(live_check) or not any(live_check)) else False


def is_cautious_fair(qbaf_collection: list[QBAFramework],
                     topic_set: list[str],
                     threshold: float) -> bool:

    safe_check =[is_safe(qbaf_collection, [x], threshold) for x in topic_set]
    live_check =[is_live(qbaf_collection, [x], threshold) for x in topic_set]

    return True if ((any(safe_check) and all(live_check)) or not any(safe_check)) else False


def calculate_gini_fairness(qbaf_collection: list[QBAFramework],
                            topic_set: list[str],
                            threshold: float) -> list:

    """
      Calculates the Gini oscillation fainress of the topic set.

      Args:
        qbaf_collection (list[QBAFramework]): The collection of QBAFs.
        topic_set (list[str]): The considered set of arguments.
        threshold (float): The justification threshold.
      Returns:
        float: returns the Gini oscillation fairness score.

    """

    x_axis = [x for x in range(0, len(topic_set)+1)]

    # Calculating the safety curve
    sorted_oscillations = sorted(number_of_oscillations(qbaf_collection, topic_set, threshold).items(), key=lambda item: item[1])
    safety_curve = [0]
    for x in sorted_oscillations:
      safety_curve.append(safety_curve[-1] + int(x[1]))

    # Calculating the fairness line
    fairness_line = np.linspace(0, safety_curve[-1], len(topic_set)+1)

    # Calulating Gini fairness
    area_enclosed = np.trapezoid(np.abs(fairness_line - safety_curve), x_axis)
    gini_fairness = (2 / (1 + math.e ** (-area_enclosed))) - 1

    # Plotting the safety curve and fairness line
    #plt.xticks(x_axis, ['0']+[x[0] for x in sorted_oscillations])
    #plt.plot(x_axis, safety_curve, label = 'Safety Curve', color='green', marker = 's')
    #plt.plot(x_axis, fairness_line, label = 'Fairness Line', linestyle = 'dashed', color='red')
    #plt.fill_between(x_axis, safety_curve, fairness_line, alpha=0.4)
    #plt.legend()
    #plt.show()

    return gini_fairness


def calculate_shannon_fairness(qbaf_collection: list[QBAFramework],
                               topic_set: list[str],
                               threshold: float) -> list:

    """
      Calculates the Shannon oscillation fainress of the topic set.

      Args:
        qbaf_collection (list[QBAFramework]): The collection of QBAFs.
        topic_set (list[str]): The considered set of arguments.
        threshold (float): The justification threshold.
      Returns:
        float: returns the Shannon oscillation fairness score.

    """

    x_axis = [x for x in range(0, len(topic_set)+1)]


    # Calculating the oscillation probability
    oscillations = number_of_oscillations(qbaf_collection, topic_set, threshold)
    if all(oscillations[x] == 0 for x in oscillations.keys()):
      return 1
    sum_of_oscillations = sum([x[1] for x in oscillations.items()])
    oscillation_probability = {x: oscillations[x]/sum_of_oscillations for x in oscillations.keys()}

    # Calculating the information coefficient
    info_coefficient = {x: (-math.log(oscillation_probability[x])/math.log(len(topic_set))) for x in oscillations.keys() if oscillation_probability[x] > 0}
    shannon_fairness = sum([oscillation_probability[x] * info_coefficient[x] for x in info_coefficient.keys()])


    return shannon_fairness
