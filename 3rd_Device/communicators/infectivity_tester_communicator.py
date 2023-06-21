import pickle
import socket
import sys

from communicators.abstract_communicator import AbstractCommunicator
from packages.infectivity_request import *
from packages.infectivity_response import *
from packages.abstract_package import AbstractPackage
from logger.logger import Logger

class InfectivityTesterCommunicator(AbstractCommunicator):
    _MAX_BYTES = 16384
    _TESTER_PORT = 5003
    _INFECTIVITY_SERVER_PORT = 5000

    def __init__(self, host: str, port: int, logger:Logger) -> None:
        self.__host = host
        self.__port = port
        self.__server_socket = None
        self.__logger = logger


    @staticmethod
    def read_data(socket_c: socket, logger:Logger = None) -> AbstractPackage:
        data = None
        try:
            data_bytes = socket_c.recv(InfectivityTesterCommunicator._MAX_BYTES)
            data = pickle.loads(data_bytes)
            if logger is not None:
                logger.info("Message received")
        except:
            data = None
            if logger is not None:
                logger.error("Message failed to receive")
        return data

    @staticmethod
    def send_data(socket_c: socket, package: AbstractPackage, logger:Logger = None):
        try:
            pack_bytes = pickle.dumps(package)
            socket_c.sendall(pack_bytes)
            if logger is not None:
                logger.info("Message sent")
            return 0
        except:
            if logger is not None:
                logger.error("Message failed to send")
            return 1

    def connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.__logger.info("Client socket created!")
        except socket.error:
            self.__logger.error("Failed to create socket! Error :" + str(sys.exc_info()[1]))
            return -1

        try:
            # Connect to remote server
            self.__logger.info("Initiate connection to %s:%s ..." % (self.__host, str(self.__port)))
            s.connect((self.__host, self.__port))
            self.__logger.info("Connected to the server!")
            self.__server_socket = s
            return 0
        except:
            self.__logger.error("Failed to connect to the server! Error :" + str(sys.exc_info()[1]))
            return -1

    def send_response(self,response:InfectivityResponse):
        try:
            resp_bytes = pickle.dumps(response)
            self.__server_socket.sendall(resp_bytes)
            self.__logger.info("Response sent")
            return 0
        except Exception as e:
            self.__logger.error("Response failed to send: %s" % (e))
            return 1

    def read_response(self):
        response = None
        try:
            resp_bytes = self.__server_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            response = pickle.loads(resp_bytes)
            self.__logger.info("Response read")
        except Exception as e:
            self.__logger.error("Response failed to receive: %s" % (e))
            response=None
        return response

    def send_request(self,request:InfectivityRequest):
        try:
            resp_bytes = pickle.dumps(request)
            self.__server_socket.sendall(resp_bytes)
            self.__logger.info("Request sent")
            return 0
        except Exception as e:
            self.__logger.error("Request failed to send: %s" %(e))
            return 1

    def read_request(self):
        request = None
        try:
            resq_bytes = self.__server_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            request = pickle.loads(resq_bytes)
        except:
            request = None
        return request

    @staticmethod
    def read_request_socket(serv_socket:socket):
        request = None
        try:
            resq_bytes = serv_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            request = pickle.loads(resq_bytes)
        except:
            request = None
        return request