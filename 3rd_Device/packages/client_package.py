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
