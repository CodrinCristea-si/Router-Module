from enum import Enum

class IntefectivityTypes(Enum):
    UNKNOWN = 0
    UNINFECTED = 1
    SUSPICIOUS = 2
    INFECTED_MINOR = 3
    INFECTED_MAJOR = 4
    INFECTED_SEVER = 5
    DEFAULT = SUSPICIOUS