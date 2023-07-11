

class AbstractPackage:

    def __init__(self, type, payload):
        self._type = type
        self._payload = payload
        # self.__payload_size = payload_size , payload_size:int = 1024

    @property
    def type(self):
        return self._type

    @property
    def payload(self):
        return self._payload

    def __str__(self):
        string = f'$-type:%s, payload:%s$+' % (self._type, self._payload)
        string = string.replace("$-", "{")
        string = string.replace("$+", "}")
        return string