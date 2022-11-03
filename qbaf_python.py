from typing import Union

class QBAFArgument:
    """ This class represent an argument of a QBAFramework.
    """
    def __init__(self, name: str, description=""):
        """ Initializator of the class QBAFArgument.

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

    def __repr__(self) -> str:
        """ Return the string representing the object with format QBAFArgument(<name>).

        Returns:
            str: The string representing the object
        """
        return self.__str__()


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
        """ Return the patients that undergo the effect of a certain action (e.g. attack, support)
            initiated by the agent.

        Args:
            agent (QBAFArgument): The initiator of the action

        Returns:
            list: The list of QBAFArgment that undergo the effect of the action
        """
        if agent in self.__agent_patients:
            return list(self.__agent_patients[agent])
        return []

    def agents(self, patient: QBAFArgument) -> list:
        """ Return the agents that initiate a certain action (e.g. attack, support)
            which effects are undergone by the patient.

        Args:
            patient (QBAFArgument): The entity undergoing the effect of the action

        Returns:
            list: The list of QBAFArgment that initiate the action
        """
        if patient in self.__patient_agents:
            return list(self.__patient_agents[patient])
        return []

    def contains(self, agent: QBAFArgument, patient: QBAFArgument) -> bool:
        """ Return whether or not exists the relation (agent, patient) in this instance.

        Args:
            agent (QBAFArgument): The initiator of an action
            patient (QBAFArgument): The entity undergoing the effect of an action

        Returns:
            bool: True if the relation exists. Otherwise, False
        """
        return (agent, patient) in self.__relations

    def add(self, agent: QBAFArgument, patient: QBAFArgument):
        """ Add the relation (agent, patient) to this instance.

        Args:
            agent (QBAFArgument): The initiator of an action
            patient (QBAFArgument): The entity undergoing the effect of an action
        """
        if (agent, patient) not in self.__relations:
            self.__relations.add((agent, patient))
            if agent not in self.__agent_patients:
                self.__agent_patients[agent] = set()
            self.__agent_patients[agent].add(patient)
            if patient not in self.__patient_agents:
                self.__patient_agents[patient] = set()
            self.__patient_agents[patient].add(agent)

    def remove(self, agent: QBAFArgument, patient: QBAFArgument):
        """ Remove the relation (agent, patient) from this instance.

        Args:
            agent (QBAFArgument): The initiator of an action
            patient (QBAFArgument): The entity undergoing the effect of an action
        """
        if (agent, patient) in self.__relations:
            self.__relations.remove((agent, patient))
            self.__agent_patients[agent].remove(patient)
            self.__patient_agents[patient].remove(agent)

    @property
    def relations(self) -> set:
        """ Return a new set of the relations (Agent, Patient) that exist in this instance.

        Returns:
            set: A set of tuples (Agent: QBAFArgument, Patient: QBAFArgument)
        """
        return self.__relations.copy()
    
    def __copy__(self):
        """ Return a copy of this instance.
            New references are created for the copy, except for the QBAFArgument objects.

        Returns:
            QBAFARelations: A copy of this QBAFARelations instance
        """
        return QBAFARelations(self.__relations)

    def __str__(self) -> str:
        """ String representation of an instance of QBAFARelations.

        Returns:
            str: The string represtation
        """
        return f'QBAFARelations{self.__relations}'

    def __repr__(self) -> str:
        """ String representation of an instance of QBAFARelations.

        Returns:
            str: The string represtation
        """
        return self.__str__()