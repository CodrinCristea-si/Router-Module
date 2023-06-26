

class Heuristic:
    def __init__(self,filename, path, name, type, requirements):
        self.__filename = filename
        self.__path = path
        self.__name = name
        self.__type = type
        self.__requirements = requirements

    @property
    def filename(self):
        return self.__filename

    @property
    def path(self):
        return self.__path

    @property
    def name(self):
        return self.__name

    @property
    def type(self):
        return self.__type

    @property
    def requirements(self):
        return self.__requirements

    def __eq__(self, obj):
        return isinstance(obj,Heuristic) and \
            obj.requirements == self.__requirements and \
            obj.type == self.__type and \
            obj.name == self.__name and \
            obj.path == self.__path and \
            obj.filename == self.__filename

    def __neg__(self,obj):
        return not self == obj

    def __hash__(self):
        return hash((self.__type, self.__name, self.__path, self.__filename))

    def __str__(self):
        return f"[name %s, path %s, filename %s, type %d, requirements %s]" %(self.__name, self.__path,self.__filename,self.__type,str(self.__requirements))