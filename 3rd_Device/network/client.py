import socket
import pickle

from packages.client_package import Package, PackageType
from communicators.client_server_communicator import ClientServerCommunicator
from logger.logger import *


class Client:

    def __init__(self, host: str, port: int, logger:Logger) -> None:
        self.__host = host
        self.__port = port
        self.__server_socket = None
        self.__logger = logger

    def connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.__logger.info("Client socket created!")
        except socket.error:
            self.__logger.error("Failed to create socket! Error :" + str(sys.exc_info()[1]))
            return -1

        # print('# Getting remote IP address')
        # try:
        #     remote_ip = socket.gethostbyname(self.__host)
        # except socket.gaierror:
        #     print('Hostname could not be resolved. Exiting')
        #     sys.exit()

        try:
            # Connect to remote server
            # print('# Connecting to server, ' + self.__host + ' (' + str(self.__port) + ')')
            self.__logger.info("Initiate connection to %s:%s ..." % (self.__host, str(self.__port)))
            s.connect((self.__host, self.__port))
            self.__logger.info("Connected to the server!")
            self.__server_socket = s
            return 0
        except:
            self.__logger.error("Failed to connect to the server! Error :" + str(sys.exc_info()[1]))
            return -1

    def close_connection(self):
        try:
            if self.__server_socket is not None:
                self.__server_socket.close()
                self.__logger.info("Connection closed!")
        except:
            self.__logger.error("Failed to close connection to the server! Error :" + str(sys.exc_info()[1]))


    def send_test_package(self):
        # Send data to remote server
        response = None
        self.__logger.info("Creating TEST package ...")
        package = Package(PackageType.TEST, "0")
        bytes_package = pickle.dumps(package)
        try:
            self.__server_socket.sendall(bytes_package)
            self.__logger.info("Package sent to the server! Awaiting response ... ")
            self.__server_socket.settimeout(3)
            bytes_response = self.__server_socket.recv(Package.MAX_BYTES)
            response = pickle.loads(bytes_response)
            self.__logger.info("Server response: " + str(response))
        except:
            self.__logger.error('Communication failed! Error: ' + str(sys.exc_info()[1]))
            response = None
        return response

    def check_for_valid_test_package(self,package:Package):
        return package is not None and package.type == PackageType.TEST and package.payload == "1"

    def send_scan_package(self):
        # Send data to remote server
        response = None
        self.__logger.info("Creating SCAN package ...")
        package = Package(PackageType.SCAN, "")
        try:
            self.__logger.info("Creating communicator ...")
            ClientServerCommunicator.send_data(self.__server_socket,package)
            self.__logger.info("communicator send data!")
            #self.__server_socket.settimeout(43200) # 12 hours
            #response = ClientServerCommunicator.read_data(self.__server_socket)
            #self.__logger.info("communicator read data!")
            self.__logger.info("Server response: " + str(response))
        except:
            self.__logger.error('Communication failed! Error: ' + str(sys.exc_info()[1]))
            response = None
        return response