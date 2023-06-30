import socket
import pickle

from packages.infectivity_response import InfectivityResponse, InfectivityResponseType
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

    def __send_request_to_server(self,type,payload):
        req = InfectivityRequest(type, payload)
        manager = InfectivityTesterCommunicator("127.0.0.1", 5004, self._logger)
        manager.connect()
        manager.send_request(req)
        manager.close_connection()

    def __process_request_tcp(self,client_socket:socket):
        type,ip,mac,infec = RouterCommunicator.read_data(client_socket)
        if type == 1: # client connection
            self.__send_request_to_server(InfectivityRequestType.ADD_CLIENT,[ip,mac])
        if type == 2: # client disconnect
            self.__send_request_to_server(InfectivityRequestType.REMOVE_CLIENT,[ip,mac])
        if type == 3:  # client transfer
            self.__send_request_to_server(InfectivityRequestType.TRANSFER_CLIENT, [ip, mac, infec])
        if type == 5:  # lockdown on
            self.__send_request_to_server(InfectivityRequestType.LOCKDOWN_SETTINGS, [True])
        if type == 6:  # lockdown off
            self.__send_request_to_server(InfectivityRequestType.LOCKDOWN_SETTINGS, [False])
        if type == 7:  # automatic on
            self.__send_request_to_server(InfectivityRequestType.AUTOMATIC_SETTINGS, [True])
        if type == 8:  # automatic off
            self.__send_request_to_server(InfectivityRequestType.AUTOMATIC_SETTINGS, [False])

        if isinstance(type, InfectivityRequest):  # requests coming from server
            pack = type
            if pack.type == InfectivityRequestType.ARE_YOU_AWAKE:
                pack = InfectivityResponse(InfectivityResponseType.I_AM_AWAKE, [])
                InfectivityTesterCommunicator.send_data(client_socket, pack, self._logger)
                client_socket.close()
                return
            rc = RouterCommunicator("192.168.1.220", 7895, self._logger)
            rc.connect()
            if pack.type == InfectivityRequestType.TRANSFER_CLIENT:
                rc.send_request([3,pack.payload])
            if pack.type == InfectivityRequestType.LOCKDOWN_SETTINGS:
                if pack.payload[0]:
                    rc.send_request([5, None])
                elif not pack.payload[0]:
                    rc.send_request([6, None])
            if pack.type == InfectivityRequestType.AUTOMATIC_SETTINGS:
                if pack.payload[0]:
                    rc.send_request([7, None])
                elif not pack.payload[0]:
                    rc.send_request([8, None])
            rc.close_connection()

        client_socket.close()
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
