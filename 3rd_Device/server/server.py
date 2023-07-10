import multiprocessing
import sys
import socket
import traceback
from logger.logger import Logger

class Server:
    _MAIN_IP = "192.168.1.2"
    _MAX_BYTES = 4096
    _MAX_BYTES_UDP = 9216
    def __init__(self,host:str, port:int, logger:Logger):
        self._port = port
        self._host = host
        self._listeners = 5
        self._logger = logger
        self._server_socket = None
        self._server_socket_tcp = None
        self._server_socket_udp = None

    def handle_request(self,client_socket:socket):
        # Handle the client request
        self._handle_request_udp(client_socket)

    def _handle_request_tcp(self,client_socket:socket):
        raise NotImplementedError("It should be implemented")

    def _handle_request_udp(self,client_data:bytes):
        raise NotImplementedError("It should be implemented")

    def run_server(self):
        self.run_tcp_server(self._port)

    def run_tcp_server(self, port: int):
        try:
            #print("am pornit")
            # Create a server socket
            self._logger.info("Initializing Server TCP socket ...")
            self._server_socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._logger.info("Server socket created")
            self._server_socket_tcp.bind((self._host, port))
            self._server_socket_tcp.listen(self._listeners)
            self._logger.info("Server socket bind complete")

            # Waiting for clients to connect
            while True:
                #self._logger.info("Server waiting for connection on %s:%s ..." % (self._host, str(port)))
                client_socket, client_address = self._server_socket_tcp.accept()
                #self._logger.info(f"Client with IP %s has connected" % str(client_address))
                self.handle_request(client_socket)
        except Exception as ex:
            traceback.print_exc()
            self._logger.error("Error: " + str(sys.exc_info()[1]))

    def run_udp_server(self, port: int):
        try:
            #print("am pornit")
            # Create a server socket
            self._logger.info("Initializing Server UDP socket ...")
            self._server_socket_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._logger.info("Server socket created")
            self._server_socket_udp.bind((self._host, port))
            self._logger.info("Server socket bind complete")

            # Waiting for clients to connect
            while True:
                #self._logger.info("Server waiting for messages on %s:%s ..." % (self._host, str(port)))
                client_data, client_address = self._server_socket_udp.recvfrom(Server._MAX_BYTES_UDP)
                #self._logger.info(f"Client with IP %s has sent a message" % str(client_address))
                self._handle_request_udp(client_data)
        except Exception as ex:
            traceback.print_exc()
            self._logger.error("Error: " + str(sys.exc_info()[1]))

    def stop_server(self):
        if self._server_socket_udp is not None:
            self._server_socket_udp.shutdown(socket.SHUT_RDWR)
            self._server_socket_udp.close()
        if self._server_socket_tcp is not None:
            self._server_socket_tcp.shutdown(socket.SHUT_RDWR)
            self._server_socket_tcp.close()
        if self._server_socket is not None:
            self._server_socket.shutdown(socket.SHUT_RDWR)
            self._server_socket.close()

