

class QBAFArgument:
    def __init__(self, name, description="", initial_weight=0.0):
        self.__name = name
        self.description = description
        self.initial_weight = initial_weight
        self.final_weight = 0.0

    @property
    def name(self):
        return self.__name

    def __str__(self) -> str:
        return f'QBAFArgument({self.name})'
