import multiprocessing
import sys

from network.server import Server
from logger.logger import Logger


class DaemonServer(Server):

    def __init__(self, host: str = "localhost", port: int = 4896):
        super().__init__(host, port)

    def __deploy_daemon(self):
        self._logger = Logger()
        self.run_server()


    def __make_daemon_immortal(self):
        sys.stdin.close()
        sys.stdout.close()
        sys.stderr.close()

    # def run_as_daemon(self):
    #     # Create a daemon context and run the server in it
    #     with daemon.DaemonContext():
    #         self.__deploy_daemon()

    def run_as_daemon(self) -> int:
        # Create a daemon context and run the server in it
        daemon_process = multiprocessing.Process(target= self.__deploy_daemon)
        daemon_process.daemon = True
        daemon_process.start()
        self._logger.info("Daemon with pid %s started" % daemon_process.pid)
        #self.__make_daemon_immortal()
        return daemon_process.pid
