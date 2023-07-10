import json
from enum import Enum
from packages.abstract_package import AbstractPackage


class PackageType(Enum):
    TEST = 0,
    SCAN = 1,
    RESULTS = 2,
    INIT = 3


class Package(AbstractPackage):
    MAX_BYTES = 1024


    def __init__(self, type: PackageType = PackageType.TEST, payload: str = ""):
        super().__init__(type, payload)
        self._type = type
        self._payload = payload
        # self.__payload_size = payload_size , payload_size:int = 1024

    @property
    def type(self) -> PackageType:
        return self._type

    @property
    def payload(self) -> str:
        return self._payload

    # @property
    # def payload_size(self) -> int:
    #     return self.__payload_size

    def __str__(self):
        string = f'$-type:%s, payload:%s$+' % (self._type, self._payload)
        string = string.replace("$-", "{")
        string = string.replace("$+", "}")
        return string