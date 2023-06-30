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
    CHECK_RESULTS = 11,
    ADD_HEURISTIC = 12,
    GET_LAST_MINUTE_PACKAGES = 13,
    GET_CONNECTED_CLIENTS = 14,
    GET_LAST_MINUTE_TEST = 15,
    ADD_PACKAGE = 16,
    GET_SAMPLES = 17,
    LOCKDOWN_SETTINGS = 18,
    AUTOMATIC_SETTINGS = 19,
    GET_ALL_RUNNING_TESTS = 20,
    GET_NETWORK_CONFIGURATION = 21,
    REACH_CLIENT = 22,
    TRANSFER_CLIENT_EXTENDED = 23,
    GET_TESTS_FOR_CLIENT = 24,
    GET_TESTS_FOR_CLIENT_PER_DAY = 25,
    GET_MALWARE_FOR_CLIENT_PER_DAY = 26
    GET_TEST = 27,
    GET_PACKAGES_PROTO_STATS = 28,
    GET_LAST_NR_PACKAGES = 29,
    GET_PACKAGE_PAYLOAD = 30,
    GET_ALL_HEURISTICS = 31,
    ARE_YOU_AWAKE = 32,

class InfectivityRequest(AbstractPackage):
    def __init__(self,type:InfectivityRequestType,payload:list):
        self._type = type
        self._payload = payload
