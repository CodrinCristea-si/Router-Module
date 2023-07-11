

class Sample:
    def __init__(self, platform, category,name,count = 0, id = 0):
        self.__id = id
        self.__platform = platform
        self.__category = category
        self.__name = name
        self.__count = count

    @property
    def id(self):
        return self.__id

    @id.setter
    def id(self, value):
        self.__id = value

    @property
    def platform(self):
        return self.__platform

    @property
    def category(self):
        return self.__category

    @property
    def name(self):
        return self.__name

    @property
    def count(self):
        return self.__count