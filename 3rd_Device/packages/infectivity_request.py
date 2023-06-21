from enum import Enum

from packages.abstract_package import AbstractPackage

class InfectivityRequestType(Enum):
    ADD_CLIENT = 1,
    REMOVE_CLIENT = 2,
    TRANSFER_CLIENT = 3,
    CHECK_CLIENT = 5,
    SCAN_CLIENT = 6,
    GET_PLATFORMS = 7,
    GET_CATEGORIES = 8,
    GET_SAMPLE_STATS = 9,
    GET_ALL_CLIENTS = 10,

class InfectivityRequest(AbstractPackage):
    def __init__(self,type:InfectivityRequestType,payload:list):
        self._type = type
        self._payload = payload
