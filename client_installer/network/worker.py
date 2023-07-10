import socket

from executor.clamav_executor import ClamAVExecutor
from packages.client_package import *
from communicators.client_server_communicator import ClientServerCommunicator
from executor.clamav_executor_linux import *
from logger.logger import Logger


class Worker:
    __TEST_INIT_PAYLOAD = "0"
    __TEST_END_PAYLOAD = "1"

    def __init__(self, client_socket:socket, logger:Logger):
        self.__client_socket = client_socket
        self._logger = logger

    def __create_results_package(self,data:ScanResults) -> Package:
        payload = {"malware": data.list_malware, "ratio": data.ratio}
        return Package(PackageType.RESULTS,str(payload))

    def process_data(self,package:Package) -> Package:
        if package.type == PackageType.SCAN:
            executor = ClamAVExecutor(self._logger)
            results = ScanResults({},[])
            try:
                results = executor.execute()
            except Exception as e:
                results = ScanResults({},[])
                self._logger.info("Error %s" %(e))
            package = self.__create_results_package(results)
            return package
        if package.type == PackageType.TEST:
            if package.payload == self.__TEST_INIT_PAYLOAD:
                return Package(PackageType.TEST, self.__TEST_END_PAYLOAD)

    def process_request(self):
        self._logger.info("Begin process request")
        package = ClientServerCommunicator.read_data(self.__client_socket)
        if package.type == PackageType.TEST:
            package_to_send = self.process_data(package)
            ClientServerCommunicator.send_data(self.__client_socket,package_to_send)
            self.__client_socket.close()
        elif package.type == PackageType.SCAN:
            package_to_send = self.process_data(package)
            comm = ClientServerCommunicator("192.168.1.2",5002)
            comm.connect()
            comm.send_packs(package_to_send)
            comm.close_connection()
        self._logger.info("End process request")
        self._logger.info("Client socket closed")
