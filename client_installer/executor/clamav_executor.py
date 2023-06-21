from executor.executor import Executor
from executor.results import ScanResults
from logger.logger import Logger
from executor.clamav_executor_linux import ClamAVExecutorLinux

import platform


class ClamAVExecutor(Executor):

    def __init__(self, logger: Logger):
        super().__init__(logger)

    def execute(self) -> ScanResults:
        self._logger.info("ClamAV platform %s" % platform.system() )
        if "Linux" in platform.system() or "Linux" in platform.version():
            linux_clamav = ClamAVExecutorLinux(self._logger)
            self._logger.info("ClamAv Linux Executor delegated")
            return linux_clamav.execute()
        else:
            raise NotImplementedError("No ClamAV Executor for this system")
