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

    def __len__(self) -> int:
        """ Return the amount of relations of the instance.

        Returns:
            int: the length
        """
        return len(self.__relations)

    def __contains__(self, agent_patient: tuple) -> bool:
        """ Return whether or not exists the relation (agent, patient) in this instance.

        Args:
            agent_patient (tuple): a tuple (Agent: QBAFArgument, Patient: QBAFArgument)

        Returns:
            bool: True if the relation exists. Otherwise, False
        """
        agent, patient = agent_patient
        return self.contains(agent, patient)

    def isdisjoint(self, other) -> bool:
        """ Return whether or not this instance has no relation in common with the instance other.

        Args:
            other (QBAFARelations): other instance of QBAFARelations

        Returns:
            bool: Return True if the instances do not share any relations. Otherwise, False.
        """
        return self.__relations.isdisjoint(other.__relations)

class QBAFramework:

    def __init__(self, arguments: list, initial_weights: list, attack_relations: Union[set,list], support_relations: Union[set,list]):
        self.__arguments = set(arguments)
        self.__initial_weights = dict()
        for arg, init_weight in zip(arguments, initial_weights):
            self.__initial_weights[arg] = init_weight
        self.__attack_relations = QBAFARelations(attack_relations)
        self.__support_relations = QBAFARelations(support_relations)
        if not self.__attack_relations.isdisjoint(self.__support_relations):
            raise ValueError
        self.__final_weights = dict()
        self.__modified = True          # True if the object have been modified after calculating the final weights

    @property
    def arguments(self) -> set:
        return self.__arguments.copy()

    @property
    def attack_relations(self) -> QBAFARelations:
        return self.__attack_relations

    @property
    def support_relations(self) -> QBAFARelations:
        return self.__support_relations

    def set_initial_weight(self, argument: QBAFArgument, initial_weight: float):
        self.__modified = True
        self.__initial_weights[argument] = initial_weight
    
    def get_initial_weight(self, argument: QBAFARelations) -> float:
        return self.__initial_weights[argument]

    def add_argument(self, argument: QBAFArgument, initial_weight=0.0):
        if argument not in self.__arguments:
            self.__modified = True
            self.__arguments.add(argument)
            self.__initial_weights[argument] = initial_weight

    def remove_argument(self, argument: QBAFArgument):
        if argument in self.__arguments:
            self.__modified = True
            self.__arguments.remove(argument)
            del self.__initial_weights[argument]

    def add_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument):
        if attacker not in self.__arguments or attacked not in self.__arguments:
            raise ValueError
        if (attacker, attacked) in self.__support_relations:
            raise ValueError
        self.__modified = True
        self.__attack_relations.add(attacker, attacked)

    def remove_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument):
        self.__modified = True
        self.__attack_relations.remove(attacker, attacked)

    def add_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument):
        if supporter not in self.__arguments or supported not in self.__arguments:
            raise ValueError
        if (supporter, supported) in self.__attack_relations:
            raise ValueError
        self.__modified = True
        self.__support_relations.add(supporter, supported)

    def remove_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument):
        self.__modified = True
        self.__support_relations.remove(supporter, supported)

    def contains_argument(self, argument: QBAFArgument) -> bool:
        return argument in self.__arguments

    def contains_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument) -> bool:
        return self.__attack_relations.contains(attacker, attacked)

    def contains_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument) -> bool:
        return self.__support_relations.contains(supporter, supported)

    def __copy__(self):
        arguments = []
        initial_weights = []
        for arg, init_weight in self.__initial_weights.items():
            arguments.append(arg)
            initial_weights.append(init_weight)
        attack_relations = self.__attack_relations.relations
        support_relations = self.__support_relations.relations
        return QBAFramework(arguments, initial_weights, attack_relations, support_relations)

    def __connected_arguments(self, argument:QBAFArgument, visiting: set, not_visited: set) -> list:
        # If argument is being visit, do not visit it again but return it
        if argument in visiting:
            return [argument]
        # We add it to visiting
        visiting.add(argument)
        children = self.__attack_relations.patients(argument) + self.__support_relations.patients(argument)
        result = []
        for child in children:
            if child in not_visited:
                result += self.__connected_arguments(child, visiting, not_visited)
        not_visited.remove(argument)
        visiting.remove(argument)
        return result

    def isacyclic(self) -> bool:
        not_visited = self.__arguments.copy()
        while len(not_visited) > 0:
            argument = not_visited.pop()
            not_visited.add(argument)
            connected_arguments = self.__connected_arguments(argument, set(), not_visited)
            detected_cycle = len(connected_arguments) > len(set(connected_arguments))
            if detected_cycle:
                return False
        return True

    def __calculate_f_weight(self, argument: QBAFArgument) -> float:
        if argument in self.__final_weights:
            return self.__final_weights[argument]
        final_weight = self.get_initial_weight(argument)
        for attacker in self.__attack_relations.agents(argument):
            final_weight -= self.__calculate_f_weight(attacker)
        for supporter in self.__support_relations.agents(argument):
            final_weight += self.__calculate_f_weight(supporter)
        self.__final_weights[argument] = final_weight
        return final_weight

    def __calculate_final_weights(self):
        if not self.isacyclic:
            raise NotImplementedError

        self.__final_weights = dict()
        not_visited = self.__arguments.copy()
        while len(not_visited) > 0:
            argument = not_visited.pop()
            self.__calculate_f_weight(argument)

    @property
    def final_weights(self) -> dict:
        if self.__modified:
            self.__calculate_final_weights()
            self.__modified = False
        return self.__final_weights

    def are_strength_consistent(self, other, arg1: QBAFArgument, arg2: QBAFArgument) -> bool:
        if self.final_weights[arg1] < self.final_weights[arg2]:
            return other.final_weights[arg1] < other.final_weights[arg2]
        if self.final_weights[arg1] > self.final_weights[arg2]:
            return other.final_weights[arg1] > other.final_weights[arg2]
        return other.final_weights[arg1] == other.final_weights[arg2]
