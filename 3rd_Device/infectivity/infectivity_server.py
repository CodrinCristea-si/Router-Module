import sys
import pickle
import socket

from logger.logger import Logger
from server.server import Server
from concurrent.futures import ThreadPoolExecutor
from infectivity.infectivity_manager import InfectivityManager
from packages.infectivity_request import *
from packages.infectivity_response import *
from db.session import SessionMaker
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator

class InfectivityServer(Server):

    def __init__(self,ip,port,logger:Logger):
        super().__init__(ip,port,logger)
        self.__max_workers = 8
        self.__thread_pool = ThreadPoolExecutor(max_workers=self.__max_workers)
        #super().run_server()

    def __add_task_executor(self,client_socket:socket):
        self._logger.info("__add_task_executor")
        session = None
        try:
            session = SessionMaker.create_scoped_session()
            self._logger.info("Session created %s" %(session))
            infectivity_manager = InfectivityManager(self._logger,session)
            self._logger.info("manager created")
            package = InfectivityTesterCommunicator.read_request_socket(client_socket)
            if package is None:
                return
            self._logger.info("Payload of type %s" % (package.type))
            if package.type == InfectivityRequestType.ADD_CLIENT:
                self._logger.info("add client")
                ip, mac = package.payload
                infectivity_manager.new_client_connection(ip,mac)
            if package.type == InfectivityRequestType.REMOVE_CLIENT:
                self._logger.info("remove client")
                ip,mac = package.payload
                infectivity_manager.remove_client_connection(ip,mac)
            if package.type == InfectivityRequestType.GET_PLATFORMS:
                self._logger.info("get all platforms")
                plats = infectivity_manager.get_all_platforms()
                print(plats)
                response = InfectivityResponse(InfectivityResponseType.PLATFORMS,plats)
                InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
            if package.type == InfectivityRequestType.GET_CATEGORIES:
                self._logger.info("get all categories")
                cats = infectivity_manager.get_all_categories()
                print(cats)
                response = InfectivityResponse(InfectivityResponseType.CATEGORIES,cats)
                InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
            if package.type == InfectivityRequestType.GET_SAMPLE_STATS:
                self._logger.info("get sample stats")
                stats = infectivity_manager.get_sample_stats()
                print(stats)
                response = InfectivityResponse(InfectivityResponseType.SAMPLE_STATS,stats)
                InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
            if package.type == InfectivityRequestType.GET_ALL_CLIENTS:
                self._logger.info("get all clients")
                clients = infectivity_manager.get_all_clients()
                response = InfectivityResponse(InfectivityResponseType.ALL_CLIENTS, clients)
                InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
            if package.type == InfectivityRequestType.ADD_PACKAGE:
                self._logger.info("add package")
                network_pack = package.payload[0]
                infectivity_manager.add_package(network_pack)
            if package.type == InfectivityRequestType.GET_SAMPLES:
                self._logger.info("get all samples")
                samples = infectivity_manager.get_all_samples()
                response = InfectivityResponse(InfectivityResponseType.SAMPLES,samples)
                InfectivityTesterCommunicator.send_big_data(client_socket,response,self._logger)

        except Exception as e:
            self._logger.error("Error: %s" %(e))
        if session is not None:
            session.remove()


    def handle_request(self,client_socket:socket):
        self.__thread_pool.submit(self.__add_task_executor,client_socket)






