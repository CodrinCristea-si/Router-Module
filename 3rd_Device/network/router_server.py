import socket
import pickle
from server.server import Server
from communicators.router_communicator import RouterCommunicator
from logger.logger import Logger
from threading import Thread
from packages.infectivity_request import *
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator


class RouterServer(Server):

    def __init__(self,host:str,port:int,logger:Logger):
        super().__init__(host,port,logger)
        self.__workers=set()

    def __process_request(self,client_socket:socket):
        type,ip,mac,infec = RouterCommunicator.read_data(client_socket)
        if type == 1: # client connection
            req = InfectivityRequest(InfectivityRequestType.ADD_CLIENT,[ip,mac])
            manager = InfectivityTesterCommunicator("127.0.0.1", 5004, self._logger)
            manager.connect()
            manager.send_request(req)
        # print("data",repr(data))

    def __clear_finished_workers(self):
        to_clear = []
        for worker in self.__workers:
            if not worker.is_alive():
                worker.join()
                self._logger.info("Thread killed")
                to_clear.append(worker)
        for elim in to_clear:
            self.__workers.remove(elim)

    def handle_request(self, client_socket: socket):
        th = Thread(target=self.__process_request, args=(client_socket,))
        self.__clear_finished_workers()
        self._logger.info("Finished workers cleared")
        self.__workers.add(th)
        self._logger.info("Thread created")
        th.start()
        self._logger.info("Thread %s started" % (th.native_id))

