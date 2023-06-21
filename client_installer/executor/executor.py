from abc import ABC
from executor.results import *
from logger.logger import Logger


class Executor(ABC):

    def __init__(self, logger:Logger):
        self._logger = logger

    def execute(self) -> ScanResults:
        raise NotImplementedError("This is an abstract method that should be override")