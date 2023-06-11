import sys
import pickle
import socket

from logger.logger import Logger
from server.server import Server
from concurrent.futures import ThreadPoolExecutor
from infectivity.infectivity_manager import InfectivityManager
from packages.infectivity_request import *
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
        except Exception as e:
            self._logger.error("Error: %s" %(e))
        if session is not None:
            session.remove()


    def handle_request(self,client_socket:socket):
        self.__thread_pool.submit(self.__add_task_executor,client_socket)






