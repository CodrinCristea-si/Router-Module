import multiprocessing
import sys

from logger.logger import Logger
from network.worker import *


class Server:

    def __init__(self,host:str, port:int, logger:Logger):
        self._port = port
        self._host = host
        self._listeners = 5
        self._logger = logger
        self._server_socket = None

    def handle_request(self,client_socket:socket):
        # Handle the client request
        try:
            worker = Worker(client_socket, self._logger)
            worker.process_request()
        except Exception:
            error = str(sys.exc_info()[1])
            self._logger.error(error)

    def run_server(self):
        try:
            #print("am pornit")
            # Create a server socket
            self._logger.info("Initializing Server socket ...")
            self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._logger.info("Server socket created")
            self._server_socket.bind((self._host, self._port))
            self._server_socket.listen(self._listeners)
            self._logger.info("Server socket bind complete")

            # Waiting for clients to connect
            while True:
                self._logger.info("Server waiting for connection on %s:%s ..." % (self._host, str(self._port)))
                client_socket, client_address = self._server_socket.accept()
                self._logger.info(f"Client with IP %s has connected" % str(client_address))
                self.handle_request(client_socket)
        except Exception as ex:
            self._logger.error("Error: " + str(sys.exc_info()[1]))

    def stop_server(self):
        self._server_socket.shutdown(socket.SHUT_RDWR)
        self._server_socket.close()
