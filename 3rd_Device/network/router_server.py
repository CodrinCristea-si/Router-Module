import socket
import pickle
from server.server import Server
from communicators.router_communicator import RouterCommunicator
from logger.logger import Logger
from threading import Thread
from packages.infectivity_request import *
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator
from domain.package import Package as PackageDOM


class RouterServer(Server):

    def __init__(self,host:str,port_tcp:int,port_udp:int,logger:Logger):
        super().__init__(host,port_tcp,logger)
        self.__port_tcp = port_tcp
        self.__port_udp = port_udp
        self.__th_tcp = None
        self.__th_udp = None
        self.__workers_tcp = set()
        self.__workers_udp = set()

    def __process_request_tcp(self,client_socket:socket):
        type,ip,mac,infec = RouterCommunicator.read_data(client_socket)
        if type == 1: # client connection
            req = InfectivityRequest(InfectivityRequestType.ADD_CLIENT,[ip,mac])
            manager = InfectivityTesterCommunicator("127.0.0.1", 5004, self._logger)
            manager.connect()
            manager.send_request(req)
            manager.close_connection()
        # print("data",repr(data))

    def __process_request_udp(self, client_data: bytes):
        network_package = RouterCommunicator.read_data(client_data)
        print(network_package)
        req = InfectivityRequest(InfectivityRequestType.ADD_PACKAGE,[network_package])
        manager = InfectivityTesterCommunicator("127.0.0.1", 5004, self._logger)
        manager.connect()
        manager.send_request(req)
        manager.close_connection()

    def run_server(self):
        # print(self.__port_tcp, self.__port_udp)
        self.__th_tcp = Thread(target=self.run_tcp_server, args=(self.__port_tcp,))
        self.__th_udp = Thread(target=self.run_udp_server, args=(self.__port_udp,))
        self.__th_tcp.start()
        self.__th_udp.start()

    def __clear_finished_workers(self, workers):
        to_clear = []
        for worker in workers:
            if not worker.is_alive():
                worker.join()
                self._logger.info("Thread killed")
                to_clear.append(worker)
        for elim in to_clear:
            workers.remove(elim)

    def _handle_request_udp(self, client_data: bytes):
        th = Thread(target=self.__process_request_udp, args=(client_data,))
        self.__clear_finished_workers(self.__workers_tcp)
        self._logger.info("Finished workers cleared")
        self.__workers_udp.add(th)
        self._logger.info("Thread created")
        th.start()
        self._logger.info("Thread %s started" % (th.native_id))

    def handle_request(self, client_socket: socket):
        th = Thread(target=self.__process_request_tcp, args=(client_socket,))
        self.__clear_finished_workers(self.__workers_tcp)
        self._logger.info("Finished workers cleared")
        self.__workers_tcp.add(th)
        self._logger.info("Thread created")
        th.start()
        self._logger.info("Thread %s started" % (th.native_id))

    def stop_server(self):
        super().stop_server()
        if self.__th_tcp is not None:
            self.__th_tcp.join()
        if self.__th_udp is not None:
            self.__th_udp.join()
