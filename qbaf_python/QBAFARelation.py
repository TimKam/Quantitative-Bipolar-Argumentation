from typing import Union
from .argument import QBAFArgument

class QBAFARelations:
    """ Class representing a set of Relations (Agent, Patient) of type QBAFArgument.
    Every Relation has an Agent (the initiator of an action)
    and a Patient (the entity undergoing the effect of an action).
    Example of (Agent, Patient): (Attacker, Attacked) or (Supporter, Supported).
    """
    def __init__(self, relations: Union[list, set]):
        """ Initializator of the class QBAFARelations.

        Args:
            relations (Union[list, set]): A collection of tuples (Agent: QBAFArgument, Patient: QBAFArgument)
        """
        self.__relations = set(relations)   # set of tuples (Agent, Patient)
        self.__agent_patients = dict()      # dictonary of (key, value) = (Agent, set of Patients)
        self.__patient_agents = dict()      # dictonary of (key, value) = (Patient, set of Agents)
        for agent, patient in self.__relations:
            self.__agent_patients.get(agent, default = set()).add(patient)
            self.__patient_agents.get(patient, default = set()).add(agent)

    def patients(self, agent: QBAFArgument) -> list:
        return list(self.__agent_patients.get(agent, default = set()))

    def agents(self, patient: QBAFArgument) -> list:
        return list(self.__patient_agents.get(patient, default = set()))

    def contains(self, agent: QBAFArgument, patient: QBAFArgument) -> bool:
        return (agent, patient) in self.__relations

    def add(self, agent: QBAFArgument, patient: QBAFArgument):
        if (agent, patient) not in self.__relations:
            self.__relations.add((agent, patient))
            self.__agent_patients.get(agent, default = set()).add(patient)
            self.__patient_agents.get(patient, default = set()).add(agent)

    def remove(self, agent: QBAFArgument, patient: QBAFArgument):
        if (agent, patient) in self.__relations:
            self.__relations.remove((agent, patient))
            self.__agent_patients.get(agent, default = set()).remove(patient)
            self.__patient_agents.get(patient, default = set()).remove(agent)

    @property
    def relations(self) -> set:
        return self.__relations.copy()

    def __str__(self) -> str:
        return f'QBAFARelations{self.__relations}'
    
    def __copy__(self):
        return QBAFARelations(self.__relations)