import json
from enum import Enum


class PackageType(Enum):
    TEST = 0,
    SCAN = 1,
    RESULTS = 2,
    INIT = 3


class Package:

    MAX_BYTES = 1024

    def __init__(self, type:PackageType = PackageType.TEST, payload: str = ""):
        self.__type = type
        self.__payload = payload
        # self.__payload_size = payload_size , payload_size:int = 1024

    @property
    def type(self) -> PackageType:
        return self.__type

    @property
    def payload(self) -> str:
        return self.__payload

    # @property
    # def payload_size(self) -> int:
    #     return self.__payload_size

    def __str__(self):
        string = f'$-type:%s, payload:%s$+' % (self.__type, self.__payload)
        string = string.replace("$-", "{")
        string = string.replace("$+", "}")
        return string
