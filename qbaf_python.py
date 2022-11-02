from typing import Union

class QBAFArgument:
    """This class represent an argument of a QBAFramework.
    """
    def __init__(self, name: str, description=""):
        """Initializator of the class QBAFArgument.

        Args:
            name (str): The name that will be used as identifier of the argument
            description (str, optional): The description of the argument. Defaults to "".
        """
        self.__name = name
        self.description = description

    @property
    def name(self) -> str:
        """ Return the name (id) of the argument

        Returns:
            str: The name of the argument
        """
        return self.__name

    def __eq__(self, other) -> bool:
        """ Return true if both instances have the same name (id).

        Args:
            other (QBAFArgument): The object to compare with

        Returns:
            bool: True if both instances have the same name (id). False, otherwise
        """
        return self.name == other.name

    def __hash__(self) -> int:
        """ Return the hash value of the name (id).

        Returns:
            int: The hash value
        """
        return hash(self.name)

    def __str__(self) -> str:
        """ Return the string representing the object with format QBAFArgument(<name>).

        Returns:
            str: The string representing the object
        """
        return f'QBAFArgument({self.name})'


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
            if agent not in self.__agent_patients:
                self.__agent_patients[agent] = set()
            self.__agent_patients[agent].add(patient)
            if patient not in self.__patient_agents:
                self.__patient_agents[patient] = set()
            self.__patient_agents[patient].add(agent)

    def patients(self, agent: QBAFArgument) -> list:
        if agent in self.__agent_patients:
            return list(self.__agent_patients[agent])
        return []

    def agents(self, patient: QBAFArgument) -> list:
        if patient in self.__patient_agents:
            return list(self.__patient_agents[patient])
        return []

    def contains(self, agent: QBAFArgument, patient: QBAFArgument) -> bool:
        return (agent, patient) in self.__relations

    def add(self, agent: QBAFArgument, patient: QBAFArgument):
        if (agent, patient) not in self.__relations:
            self.__relations.add((agent, patient))
            if agent not in self.__agent_patients:
                self.__agent_patients[agent] = set()
            self.__agent_patients[agent].add(patient)
            if patient not in self.__patient_agents:
                self.__patient_agents[patient] = set()
            self.__patient_agents[patient].add(agent)

    def remove(self, agent: QBAFArgument, patient: QBAFArgument):
        if (agent, patient) in self.__relations:
            self.__relations.remove((agent, patient))
            self.__agent_patients[agent].remove(patient)
            self.__patient_agents[patient].remove(agent)

    @property
    def relations(self) -> set:
        return self.__relations.copy()

    def __str__(self) -> str:
        return f'QBAFARelations{self.__relations}'
    
    def __copy__(self):
        return QBAFARelations(self.__relations)