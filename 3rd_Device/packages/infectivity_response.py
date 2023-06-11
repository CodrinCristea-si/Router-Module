from enum import Enum

from packages.abstract_package import AbstractPackage

class InfectivityResponseType(Enum):
    TEST_RESULTS = 1,
    STATUS_AVAILABLE = 2,
    STATUS_UNAVAILABLE =3


class InfectivityResponse(AbstractPackage):
    def __init__(self,type:InfectivityResponseType,payload:list):
        self._type = type
        self._payload = payload