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

    def __eq__(self, other) -> bool:
        return self.__relations == other.__relations

class QBAFramework:
    """ This class represents a Quantitative Bipolar Argumentation Framework (QBAF).
        A QBAF is is a quadruple (Args,τ,Att,Supp) consisting of a set of arguments Args, 
        an attack relation Att ⊆ Args x Args, a support relation Supp ⊆ Args x Args and
        a total function τ : Args → I that assigns the initial strength τ(a) to every a ∈ Args.
    """
    def __init__(self, arguments: list, initial_weights: list, attack_relations: Union[set,list], support_relations: Union[set,list]):
        """ Init function of a QBAFramework.

        Args:
            arguments (list): a list of QBAFArgument
            initial_weights (list): a list of floats corresponding to the arguments
            attack_relations (Union[set,list]): a collection of tuples (attacker: QBAFArgument, attacked: QBAFArgument)
            support_relations (Union[set,list]): a collection of tuples (supporter: QBAFArgument, supported: QBAFArgument)

        Raises:
            ValueError: An argument in the attack or support relations is not in the list of arguments.
            ValueError: The attack relations and the support relations are not disjoint
        """
        self.__arguments = set(arguments)
        self.__initial_weights = dict()
        for arg, init_weight in zip(arguments, initial_weights):
            self.__initial_weights[arg] = init_weight
        self.__attack_relations = QBAFARelations(attack_relations)
        self.__support_relations = QBAFARelations(support_relations)
        # Checking its integrity
        for attacker, attacked in self.__attack_relations:
            if attacker not in self.__arguments or attacked not in self.__arguments:
                raise ValueError
        for supporter, supported in self.__attack_relations:
            if supporter not in self.__arguments or supported not in self.__arguments:
                raise ValueError
        if not self.__attack_relations.isdisjoint(self.__support_relations):
            raise ValueError
        # Adding support variables
        self.__final_weights = dict()
        self.__modified = True          # True if the object have been modified after calculating the final weights

    @property
    def arguments(self) -> set:
        """ Return a copy of the arguments of the instance.

        Returns:
            set: a set of QBAFArgument
        """
        return self.__arguments.copy()

    @property
    def attack_relations(self) -> QBAFARelations:
        """ Return the attack relations of the instance.

        Returns:
            QBAFARelations: an instance of QBAFARelations
        """
        return self.__attack_relations

    @property
    def support_relations(self) -> QBAFARelations:
        """ Return the support relations of the instance.

        Returns:
            QBAFARelations: an instance of QBAFARelations
        """
        return self.__support_relations

    @property
    def initial_weights(self) -> dict:
        """ Return a copy of the initial weights.

        Returns:
            dict: a dictionary (argument: QBAFArgument, initial_weight: float)
        """
        return self.__initial_weights.copy()

    def modify_initial_weight(self, argument: QBAFArgument, initial_weight: float):
        """ Modify the initial weight of the Argument argument.

        Args:
            argument (QBAFArgument): the argument to be modified
            initial_weight (float): the new value of initial weight
        """
        self.__modified = True
        self.__initial_weights[argument] = initial_weight
    
    def initial_weight(self, argument: QBAFARelations) -> float:
        """ Return the initial weight of the Argument argument.

        Args:
            argument (QBAFARelations): the argument

        Returns:
            float: the initial weight
        """
        return self.__initial_weights[argument]

    def add_argument(self, argument: QBAFArgument, initial_weight=0.0):
        """ Add an Argument to the Framework. If it exists already it does nothing.

        Args:
            argument (QBAFArgument): the argument
            initial_weight (float, optional): the initial weight of the argument. Defaults to 0.0.
        """
        if argument not in self.__arguments:
            self.__modified = True
            self.__arguments.add(argument)
            self.__initial_weights[argument] = initial_weight

    def remove_argument(self, argument: QBAFArgument):
        """ Remove the Argument argument from the Framework. If it does not exist it does nothing.

        Args:
            argument (QBAFArgument): the argument
        """
        if argument in self.__arguments:
            # TODO: Check that the argument is not in attack/support relations
            self.__modified = True
            self.__arguments.remove(argument)
            del self.__initial_weights[argument]

    def add_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument):
        """ Add the Attack relation (attacker, attacked) to the Framework.

        Args:
            attacker (QBAFArgument): the argument that is attacking
            attacked (QBAFArgument): the argument that is attacked

        Raises:
            ValueError: The attacker or the attacked are not an argument in this Framework
            ValueError: There is a support relation (attacker, attacked)
        """
        if attacker not in self.__arguments or attacked not in self.__arguments:
            raise ValueError
        if (attacker, attacked) in self.__support_relations:
            raise ValueError
        self.__modified = True
        self.__attack_relations.add(attacker, attacked)

    def remove_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument):
        """ Remove the Attack relation (attacker, attacked) from the Framework

        Args:
            attacker (QBAFArgument): the argument that is attacking
            attacked (QBAFArgument): the argument that is attacked
        """
        self.__modified = True
        self.__attack_relations.remove(attacker, attacked)

    def add_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument):
        """ Add the Support relation (attacker, attacked) to the Framework.

        Args:
            supporter (QBAFArgument): the argument that is supporting
            supported (QBAFArgument): the argument that is supported

        Raises:
            ValueError: The supporter or the supported are not an argument in this Framework.
            ValueError: There is an attack relation (supporter, supported)
        """
        if supporter not in self.__arguments or supported not in self.__arguments:
            raise ValueError
        if (supporter, supported) in self.__attack_relations:
            raise ValueError
        self.__modified = True
        self.__support_relations.add(supporter, supported)

    def remove_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument):
        """ Remove the Attack relation (supporter, supported) from the Framework.

        Args:
            supporter (QBAFArgument): the argument that is supporting
            supported (QBAFArgument): the argument that is supported
        """
        self.__modified = True
        self.__support_relations.remove(supporter, supported)

    def contains_argument(self, argument: QBAFArgument) -> bool:
        """ Return whether or not the Framework contains the Argument argument.

        Args:
            argument (QBAFArgument): the argument

        Returns:
            bool: True if it is contanied. False, otherwise.
        """
        return argument in self.__arguments

    def contains_attack_relation(self, attacker: QBAFArgument, attacked: QBAFArgument) -> bool:
        """ Return whether or not the Attack relation (attacker, attacked) is contained in the Framework.

        Args:
            attacker (QBAFArgument): the argument that is attacking
            attacked (QBAFArgument): the argument that is attacked

        Returns:
            bool: True if it is contanied. False, otherwise.
        """
        return self.__attack_relations.contains(attacker, attacked)

    def contains_support_relation(self, supporter: QBAFArgument, supported: QBAFArgument) -> bool:
        """ Return whether or not the Support relation (supporter, supported) is contained in the Framework.

        Args:
            supporter (QBAFArgument): the argument that is supporting
            supported (QBAFArgument): the argument that is supported

        Returns:
            bool: True if it is contanied. False, otherwise.
        """
        return self.__support_relations.contains(supporter, supported)

    def __copy__(self):
        """ Return a copy of the Framework.

        Returns:
            QBAFramework: a new QBAFramework
        """
        arguments = []
        initial_weights = []
        for arg, init_weight in self.__initial_weights.items():
            arguments.append(arg)
            initial_weights.append(init_weight)
        attack_relations = self.__attack_relations.relations
        support_relations = self.__support_relations.relations
        return QBAFramework(arguments, initial_weights, attack_relations, support_relations)

    def __incycle_arguments(self, argument:QBAFArgument, not_visited: set, visiting=set()) -> list:
        """ Return a list with the arguments that are being attacked/supported by Argument argument (itself included)
            that are in a cycle.

        Args:
            argument (QBAFArgument): a QBAFArgument
            not_visited (set): a set of arguments that have not been visited yet (this set is modified in this function)
            visiting (set, optional): a set of arguments that are being visited in this function. Defaults to set().

        Returns:
            list: list of QBAFArgument that contain at least one cycle
        """
        # If argument is being visited, do not visit it again but return it
        if argument in visiting:
            return [argument]
        # We add it to visiting
        visiting.add(argument)
        children = self.__attack_relations.patients(argument) + self.__support_relations.patients(argument)
        result = []
        for child in children:
            if child in not_visited:
                result += self.__incycle_arguments(child, not_visited, visiting)
        not_visited.remove(argument)
        visiting.remove(argument)
        return result

    def isacyclic(self) -> bool:
        """ Return whether or not the relations of the Framework are acyclic.

        Returns:
            bool: True if there are no cycles. False, otherwise.
        """
        not_visited = self.__arguments.copy()
        while len(not_visited) > 0:
            argument = not_visited.pop()
            not_visited.add(argument)
            in_cycle_arguments = self.__incycle_arguments(argument, not_visited)
            detected_cycle = len(in_cycle_arguments) > 0
            if detected_cycle:
                return False
        return True

    def __calculate_f_weight(self, argument: QBAFArgument) -> float:
        """ Return the final weight of a specific argument.
            This function calls itself recursively. So, it only works with acyclic arguments.
            It stores all the calculated final weights in self.__final_weights.

        Args:
            argument (QBAFArgument): the QBAFArgument

        Returns:
            float: the final weight of the argument
        """
        if argument in self.__final_weights:
            return self.__final_weights[argument]
        final_weight = self.initial_weight(argument)
        for attacker in self.__attack_relations.agents(argument):
            final_weight -= self.__calculate_f_weight(attacker)
        for supporter in self.__support_relations.agents(argument):
            final_weight += self.__calculate_f_weight(supporter)
        self.__final_weights[argument] = final_weight
        return final_weight

    def __calculate_final_weights(self):
        """ Calculate the final weights of all the arguments of the Framework.
            It stores all the calculated final weights in self.__final_weights.

        Raises:
            NotImplementedError: The relations are not acyclic. There is no implementation for cyclic relations.
        """
        if not self.isacyclic():
            raise NotImplementedError

        self.__final_weights = dict()
        not_visited = self.__arguments.copy()
        while len(not_visited) > 0:
            argument = not_visited.pop()
            self.__calculate_f_weight(argument)

    @property
    def final_weights(self) -> dict:
        """ Return the final weights of arguments of the Framework.
            If the framework has been modified from the last time they were calculated
            they are calculated again. Otherwise, it returns the already calculated final weights.

        Returns:
            dict: a dictionary with (key, value): (argument: QBAFArgument, final_weight: float)
        """
        if self.__modified:
            self.__calculate_final_weights()
            self.__modified = False
        return self.__final_weights.copy()

    def final_weight(self, argument: QBAFArgument) -> float:
        """ Return the final weights of Argument argument of the Framework.
            If the framework has been modified from the last time the final weights were calculated
            they are calculated again. Otherwise, it returns the already calculated final weight.

        Args:
            argument (QBAFArgument): the argument

        Returns:
            float: the final weight
        """
        if self.__modified:
            self.__calculate_final_weights()
            self.__modified = False
        return self.__final_weights[argument]

    def are_strength_consistent(self, other, arg1: QBAFArgument, arg2: QBAFArgument) -> bool:
        """ Return whether or not a pair of arguments are strength consistent between two frameworks.

        Args:
            other (QBAFramework): a different instance of QBAFramework
            arg1 (QBAFArgument): first argument
            arg2 (QBAFArgument): second argument

        Returns:
            bool: True if they are strength consistent. False, otherwise.
        """
        if self.final_weights[arg1] < self.final_weights[arg2]:
            return other.final_weights[arg1] < other.final_weights[arg2]
        if self.final_weights[arg1] > self.final_weights[arg2]:
            return other.final_weights[arg1] > other.final_weights[arg2]
        return other.final_weights[arg1] == other.final_weights[arg2]

    def reversal(self, other, set):
        """ Return the reversal framework of self (QBF') to other (QBF) w.r.t. set ⊆ Args'∪Args.

        Args:
            other (QBAFramework): the framework that self will be reversed to
            set (set): a set of arguments that will be reversed

        Returns:
            QBAFramework: new instance of QBAFramework
        """
        if not set.issubset(self.__arguments.union(other.__arguments)):
            raise ValueError

        args = (self.__arguments.union(set)).difference(set.difference(other.__arguments))
        
        att = self.__attack_relations.copy()
        for arg in set:
            for attacked in self.__attack_relations.patients(arg):
                att.remove(arg, attacked)
            for attacked in other.__attack_relations.patients(arg):
                att.add(arg, attacked)
        
        supp = self.__support_relations.copy()
        for arg in set:
            for supported in self.__support_relations.patients(arg):
                supp.remove(arg, supported)
            for supported in other.__support_relations.patients(arg):
                supp.add(arg, supported)
        
        initial_weights = dict()
        for arg in args:
            if arg in other.__arguments.intersection(set):
                initial_weights[arg] = other.__initial_weights[arg]
            else:
                initial_weights[arg] = self.__initial_weights[arg]
        
        reversal = QBAFramework([],[],[],[])
        reversal.__arguments = args
        reversal.__attack_relations = att
        reversal.__support_relations = supp
        reversal.__initial_weights = initial_weights
        reversal.__modified = True

        return reversal

    def isSSIExplanation(self, other, set, arg1, arg2):
        """ Return True if a set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation
            of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF).

        Args:
            other (QBAFramework): a different instance of QBAFramework
            set (set): a set of arguments
            arg1 (QBAFArgument): first argument
            arg2 (QBAFArgument): second argument

        Returns:
            bool: True if it is a SSI Explanation. False, otherwise.
        """
        if self.are_strength_consistent(other, arg1, arg2):
            return len(set) == 0
        
        reversal = self.reversal(other, self.arguments.union(other.arguments).difference(set))

        return not other.are_strength_consistent(reversal, arg1, arg2)

    def isCSIExplanation(self, other, set, arg1, arg2):
        """ Return True if a set of arguments set is Counterfactual Strength Inconsistency (CSI) Explanation
            of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF).

        Args:
            other (QBAFramework): a different instance of QBAFramework
            set (set): a set of arguments
            arg1 (QBAFArgument): first argument
            arg2 (QBAFArgument): second argument

        Returns:
            bool: True if it is a CSI Explanation. False, otherwise.
        """
        
        reversal = self.reversal(other, set)

        if not other.are_strength_consistent(reversal, arg1, arg2):
            return False
        
        return self.isSSIExplanation(other, set, arg1, arg2)

    def __subsets(self, s: set, size: int) -> list:
        """ Return a list of subsets of lenght size from the set s.

        Args:
            s (set): a set
            size (int): the desired size of each subset

        Returns:
            list: the list of subsets
        """
        if len(s) == 0:
            return [set()]
        if size == 1:
            return [{sub} for sub in s]
        if size <= 0:
            return [set() for _ in range(len(s))]

        result = []
        myset = s.copy()
        for item in s:
            myset.remove(item)
            subsets = self.__subsets(myset, size-1)
            for subset in subsets:
                subset.add(item)
                result.append(subset)

            if len(myset) < size:
                break

        return result

    def __influential_arguments(self, argument:QBAFArgument, not_visited: set, visiting=set()) -> list:
        """ Return a list with the arguments that are attacking/supporting Argument argument directly or indirectly.

        Args:
            argument (QBAFArgument): a QBAFArgument
            not_visited (set): a set of arguments that have not been visited yet (this set is modified in this function)
            visiting (set, optional): a set of arguments that are being visited in this function. Defaults to set().

        Returns:
            list: list of QBAFArgument that contain at least one cycle
        """
        # If argument is being visited, do not visit it again but return it
        if argument in visiting:
            return [argument]
        # We add it to visiting
        visiting.add(argument)
        parents = self.__attack_relations.agents(argument) + self.__support_relations.agents(argument)
        result = [argument]
        for arg in parents:
            if arg in not_visited:
                result += self.__influential_arguments(arg, not_visited, visiting)
        not_visited.remove(argument)
        visiting.remove(argument)
        return result

    def __influential_arguments_set(self, arg1: QBAFArgument, arg2: QBAFArgument) -> set:
        """ Return a set with the arguments that are attacking/supporting Argument arg1 or Argument arg2, 
            directly or indirectly.

        Args:
            arg1 (QBAFArgument): a QBAFArgument
            arg2 (QBAFArgument): a QBAFArgument
        """
        not_visited = self.__arguments.copy()
        visiting = set()
        influential_arguments = self.__influential_arguments(arg1, not_visited, visiting)
        influential_arguments += self.__influential_arguments(arg2, not_visited, visiting)

        return set(influential_arguments)

    def __candidate_argument(self, other, argument: QBAFArgument) -> bool:
        """ Return True if the Argument argument is candidate for a CSI explanation, False if not.
        An Argument is candidate if it is not contained by one of the frameworks or
        it has a different initial weight or it has a different final weight between the frameworks or
        it has different relations as attacker/supporter (as attacked/supported not checked).

        Args:
            other (QBAFramework): other instance of QBAFramework
            argument (QBAFArgument): the argument

        Returns:
            bool: True if candidate, False if not
        """
        if not (argument in self.__arguments and argument in other.__arguments):
            return True
        if self.__initial_weights[argument] != other.__initial_weights[argument]:
            return True
        if self.final_weight(argument) != other.final_weight(argument):
            return True
        if self.__attack_relations.patients(argument) != other.__attack_relations.patients(argument):
            return True
        if self.__support_relations.patients(argument) != other.__support_relations.patients(argument):
            return True

        return False

    def minimalSSIExplanations(self, other, arg1: QBAFArgument, arg2: QBAFArgument) -> list:
        """ Return a list of all the sets of arguments that are minimal SSI Explanations
            of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF).

        Args:
            other (QBAFramework): a QBAFramework
            arg1 (QBAFArgument): a QBAFArgument
            arg2 (QBAFArgument): a QBAFArgument

        Returns:
            list: list of sets of arguments
        """
        empty_set = set()
        if self.isSSIExplanation(other, empty_set, arg1, arg2):
            return [empty_set]

        influential_arguments = self.__influential_arguments_set(arg1, arg2).union(other.__influential_arguments_set(arg1, arg2))
        
        candidate_arguments = set()
        for argument in influential_arguments:
            if self.__candidate_argument(other, argument):
                candidate_arguments.add(argument)

        explanations = []
        for size in range(1, len(candidate_arguments)+1):
            subsets = self.__subsets(candidate_arguments, size)
            for set in subsets:
                minimal_in_explanations = False
                for exp in explanations:
                    if exp.issubset(set):
                        minimal_in_explanations = True
                        break
                if not minimal_in_explanations and self.isSSIExplanation(other, set, arg1, arg2):
                    explanations.append(set)
        
        return explanations

    def minimalCSIExplanations(self, other, arg1: QBAFArgument, arg2: QBAFArgument) -> list:
        """ Return a list of all the sets of arguments that are minimal CSI Explanations
            of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF).

        Args:
            other (QBAFramework): a QBAFramework
            arg1 (QBAFArgument): a QBAFArgument
            arg2 (QBAFArgument): a QBAFArgument

        Returns:
            list: list of sets of arguments
        """
        empty_set = set()
        if self.isCSIExplanation(other, empty_set, arg1, arg2):
            return [empty_set]

        influential_arguments = self.__influential_arguments_set(arg1, arg2).union(other.__influential_arguments_set(arg1, arg2))
        
        candidate_arguments = set()
        for argument in influential_arguments:
            if self.__candidate_argument(other, argument):
                candidate_arguments.add(argument)

        explanations = []
        for size in range(1, len(candidate_arguments)+1):
            subsets = self.__subsets(candidate_arguments, size)
            for set in subsets:
                minimal_in_explanations = False
                for exp in explanations:
                    if exp.issubset(set):
                        minimal_in_explanations = True
                        break
                if not minimal_in_explanations and self.isCSIExplanation(other, set, arg1, arg2):
                    explanations.append(set)
        
        return explanations

    def __eq__(self, other: object) -> bool:
        return (self.__arguments == other.__arguments
            and self.__initial_weights == other.__initial_weights
            and self.__attack_relations == other.__attack_relations
            and self.__support_relations == other.__support_relations)
        