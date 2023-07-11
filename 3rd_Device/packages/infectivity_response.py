from enum import Enum

from packages.abstract_package import AbstractPackage

class InfectivityResponseType(Enum):
    TEST_RESULTS = 1,
    STATUS_AVAILABLE = 2,
    STATUS_UNAVAILABLE = 3,
    PLATFORMS = 4,
    CATEGORIES = 5,
    SAMPLE_STATS = 6,
    ALL_CLIENTS = 7,
    HEURISTIC_RESULTS = 8,
    SAMPLES = 9,
    RUNNING_TESTS = 10,
    NETWORK_CONFIGS = 11,
    CONNECTED_CLIENTS = 12,
    REACH_RESPONSE = 13,
    GENERIC_RESPONSE = 14,
    CLIENT_TESTS = 15,
    TEST_DETAILS = 16,
    I_AM_AWAKE = 17,


class InfectivityResponse(AbstractPackage):
    def __init__(self, type: InfectivityResponseType, payload: list):
        super().__init__(type, payload)
        self._type = type
        self._payload = payload