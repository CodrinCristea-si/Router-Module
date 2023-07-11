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
    _TESTER_PORT = 5002
    _INFECTIVITY_SERVER_PORT = 5004

    def __init__(self, host: str, port: int, logger:Logger) -> None:
        self.__host = host
        self.__port = port
        self.__server_socket = None
        self.__logger = logger
        self.__timeout = 3


    @staticmethod
    def read_data(socket_c: socket, logger:Logger = None) -> AbstractPackage:
        data = None
        try:
            #data_bytes = socket_c.recv(InfectivityTesterCommunicator._MAX_BYTES)
            data_bytes = InfectivityTesterCommunicator.read_raw_data(socket_c,logger)
            data = pickle.loads(data_bytes)
            if logger is not None:
                logger.info("Message received")
        except:
            data = None
            if logger is not None:
                logger.error("Message failed to receive")
        return data

    @staticmethod
    def read_big_data(socket_c: socket, logger: Logger = None) -> AbstractPackage:
        data = None
        try:
            resp_bytes_size = socket_c.recv(5)
            resp_bytes_size = int.from_bytes(resp_bytes_size, 'big')
            received_data = b''
            total_length = 0
            while True:
                chunk = socket_c.recv(InfectivityTesterCommunicator._MAX_BYTES)
                total_length += len(chunk)
                received_data += chunk
                if not chunk or total_length >= resp_bytes_size:
                    break
            data = pickle.loads(received_data)
        except Exception as e:
            data = None
            if logger is not None:
                logger.error("Message failed to receive! Error: %s"%(e))
        return data

    @staticmethod
    def read_raw_data(socket_c: socket, logger: Logger = None) -> bytes:
        data = b""
        try:
            raw_bytes_size = socket_c.recv(5)
            #print("rb",raw_bytes_size)
            raw_bytes_size = int.from_bytes(raw_bytes_size, 'big')
            #print("rn", raw_bytes_size)
            received_data = b''
            total_length = 0
            while True:
                chunk = socket_c.recv(InfectivityTesterCommunicator._MAX_BYTES)
                #print(chunk)
                total_length += len(chunk)
                #print("read",total_length, raw_bytes_size)
                received_data += chunk
                if not chunk or total_length >= raw_bytes_size:
                    break
                #print("received_data",received_data)
            data = received_data
            #print("ceva", data)
        except Exception as e:
            data = b""
            if logger is not None:
                logger.error("Message failed to receive! Error: %s"%(e))
        return data

    @staticmethod
    def send_data(socket_c: socket, package: AbstractPackage, logger:Logger = None):
        try:
            pack_bytes = pickle.dumps(package)
            InfectivityTesterCommunicator.send_raw_data(socket_c,pack_bytes,logger)
            #socket_c.sendall(pack_bytes)
            if logger is not None:
                logger.info("Message sent")
            return 0
        except:
            if logger is not None:
                logger.error("Message failed to send")
            return 1

    @staticmethod
    def send_raw_data(socket_c: socket, raw_bytes: bytes, logger:Logger = None):
        try:
            #print("w", raw_bytes)
            raw_bytes_size = len(raw_bytes)
            number_bytes = raw_bytes_size.to_bytes(5, "big")
            #print('w', number_bytes)
            socket_c.send(number_bytes)
            offset = 0
            total_length = 0
            end_data = 0
            while offset < raw_bytes_size:
                end_data = InfectivityTesterCommunicator._MAX_BYTES \
                    if total_length + InfectivityTesterCommunicator._MAX_BYTES <= raw_bytes_size else raw_bytes_size - total_length + 1
                chunk = raw_bytes[offset:offset + end_data]
                total_length += len(chunk)
                socket_c.send(chunk)
                offset += InfectivityTesterCommunicator._MAX_BYTES
                #print(total_length, offset, raw_bytes_size, end_data)
            #print(total_length, raw_bytes_size, end_data)
            return 0
        except Exception as e:
            if logger is not None:
                logger.error("Message failed to send! Error: %s" % (e))
            return 1

    @staticmethod
    def send_big_data(socket_c: socket, package: AbstractPackage, logger: Logger = None):
        try:
            resp_bytes = pickle.dumps(package)
            resp_bytes_size = len(resp_bytes)
            number_bytes = resp_bytes_size.to_bytes(5, "big")
            socket_c.send(number_bytes)
            offset = 0
            total_length = 0
            end_data = 0
            while offset < resp_bytes_size:
                end_data = InfectivityTesterCommunicator._MAX_BYTES if total_length + InfectivityTesterCommunicator._MAX_BYTES <= resp_bytes_size else resp_bytes_size - total_length + 1
                chunk = resp_bytes[offset:offset + end_data]
                total_length += len(chunk)
                #print(total_length, resp_bytes_size, end_data)
                socket_c.send(chunk)
                offset += InfectivityTesterCommunicator._MAX_BYTES
            #print(total_length, resp_bytes_size, end_data)
            return 0
        except Exception as e:
            if logger is not None:
                logger.error("Message failed to send! Error: %s"%(e))
            return 1

    def connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #self.__logger.info("Client socket created!")
        except socket.error:
            self.__logger.error("Failed to create socket! Error :" + str(sys.exc_info()[1]))
            return -1

        try:
            # Connect to remote server
            #self.__logger.info("Initiate connection to %s:%s ..." % (self.__host, str(self.__port)))
            s.connect((self.__host, self.__port))
            #self.__logger.info("Connected to the server!")
            self.__server_socket = s
            return 0
        except:
            self.__logger.error("Failed to connect to the server! Error :" + str(sys.exc_info()[1]))
            return -1

    def close_connection(self):
        if self.__server_socket is not None:
            self.__server_socket.close()

    def send_response(self,response:InfectivityResponse):
        try:
            resp_bytes = pickle.dumps(response)
            InfectivityTesterCommunicator.send_raw_data(self.__server_socket,resp_bytes,self.__logger)
            #self.__server_socket.sendall(resp_bytes)
            #self.__logger.info("Response sent")
            return 0
        except Exception as e:
            self.__logger.error("Response failed to send: %s" % (e))
            return 1

    def read_response(self):
        response = None
        try:
           # resp_bytes = self.__server_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            resp_bytes = InfectivityTesterCommunicator.read_raw_data(self.__server_socket,self.__logger)
            response = pickle.loads(resp_bytes)
            #self.__logger.info("Response read")
        except Exception as e:
            self.__logger.error("Response failed to receive: %s" % (e))
            response=None
        return response

    def read_big_response(self):
        # number_bytes = number.to_bytes(4, 'big')
        response = None
        try:
            resp_bytes_size = self.__server_socket.recv(5)
            resp_bytes_size = int.from_bytes(resp_bytes_size, 'big')

            received_data = []
            total_length = 0
            while total_length < resp_bytes_size:
                chunk = self.__server_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
                total_length += len(chunk)
                #print(total_length)
                received_data.append(chunk)
            response = pickle.loads(b''.join(received_data))
            #print("done boss")
        except Exception as e:
            self.__logger.error("Response failed to receive: %s" % (e))
            response=None
        return response

        #response = pickle.loads(resp_bytes)

        pass

    def send_big_response(self, response:InfectivityResponse):
        try:
            resp_bytes = pickle.dumps(response)
            resp_bytes_size = len(resp_bytes)
            number_bytes = resp_bytes_size.to_bytes(5,"big")
            self.__server_socket.sendall(number_bytes)
            offset = 0
            while offset < resp_bytes_size:
                chunk = resp_bytes[offset:offset + InfectivityTesterCommunicator._MAX_BYTES]
                self.__server_socket.sendall(chunk)
                offset += InfectivityTesterCommunicator._MAX_BYTES
            return 0
        except Exception as e:
            self.__logger.error("Response failed to send: %s" % (e))
            return 1

    def send_request(self,request:InfectivityRequest):
        try:
            resp_bytes = pickle.dumps(request)
            InfectivityTesterCommunicator.send_raw_data(self.__server_socket,resp_bytes,self.__logger)
            #self.__server_socket.sendall(resp_bytes)
            #self.__logger.info("Request sent")
            return 0
        except Exception as e:
            self.__logger.error("Request failed to send: %s" %(e))
            return 1

    def read_request(self):
        request = None
        try:
            #resq_bytes = self.__server_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            resq_bytes = InfectivityTesterCommunicator.read_raw_data(self.__server_socket,self.__logger)
            request = pickle.loads(resq_bytes)
        except:
            request = None
        return request

    @staticmethod
    def read_request_socket(serv_socket:socket):
        request = None
        try:
            #resq_bytes = serv_socket.recv(InfectivityTesterCommunicator._MAX_BYTES)
            resq_bytes = InfectivityTesterCommunicator.read_raw_data(serv_socket,None)
            #print("read_request_socket", resq_bytes)
            request = pickle.loads(resq_bytes)
            #print(request)
        except:
            request = None
        return request

