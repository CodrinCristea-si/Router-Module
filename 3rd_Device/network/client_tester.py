import pickle
import socket
from server.server import Server
from logger.logger import Logger
from threading import Thread

from communicators.client_server_communicator import ClientServerCommunicator
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator
from packages.client_package import Package as ClientPackage
from packages.infectivity_request import *
from packages.infectivity_response import *
from network.client import Client


class ClientTester(Server):
    __DEFAULT_PORT_CLIENTS = 5004

    def __init__(self,host:str,port:int,logger:Logger):
        super().__init__(host,port,logger)
        self.__workers = set()

    def __send_infectivity_response(self,response:InfectivityResponse):
        manager = InfectivityTesterCommunicator("127.0.0.1",5004,self._logger)
        manager.connect()
        manager.send_response(response)

    def __process_request(self,client_socket:socket):
        request = InfectivityTesterCommunicator.read_data(client_socket)
        self._logger.info("Request received")
        response = None
        if request.type == InfectivityRequestType.CHECK_CLIENT:
            host = request.payload[0]
            self._logger.info("Request of type CHECK_CLIENT for %s"%(host))
            client = Client(host, ClientTester.__DEFAULT_PORT_CLIENTS, self._logger)
            res = client.connect()
            if res == 0:
                self._logger.info("Connection to %s estamblished" %(host))
                resp = client.send_test_package()
                if resp is not None and client.check_for_valid_test_package(resp):
                    response = InfectivityResponse(InfectivityResponseType.STATUS_AVAILABLE,[host])
                    self._logger.info("Host %s check status valid" % (host))
                else:
                    response = InfectivityResponse(InfectivityResponseType.STATUS_UNAVAILABLE, [host])
                    self._logger.info("Host %s check status invalid" % (host))
            else:
                response = InfectivityResponse(InfectivityResponseType.STATUS_UNAVAILABLE, [host])
                self._logger.info("Host %s is unreachable" % (host))
            InfectivityTesterCommunicator.send_data(client_socket,response)


        #case for SCAN_CLIENT
        #self.__send_infectivity_response(response)


    def __clear_finished_workers(self):
        to_clear=[]
        for worker in self.__workers:
            if not worker.is_alive():
                worker.join()
                to_clear.append(worker)
        for elim in to_clear:
            self.__workers.remove(elim)

    def handle_request(self, client_socket: socket):
        th = Thread(target = self.__process_request,args=(client_socket,))
        self.__clear_finished_workers()
        self._logger.info("Finished workers cleared")
        self.__workers.add(th)
        self._logger.info("Thread created")
        th.start()
        self._logger.info("Thread %s started" % (th.native_id))

