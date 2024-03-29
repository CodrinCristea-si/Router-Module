import socket
import pickle

from packages.client_package import Package, PackageType
from communicators.abstract_communicator import AbstractCommunicator


class ClientServerCommunicator(AbstractCommunicator):
    __MAX_BYTES = Package.MAX_BYTES

    def __init__(self, host: str, port: int) -> None:
        self.__host = host
        self.__port = port
        self.__server_socket = None
        self.__timeout = 3


    @staticmethod
    def read_data(socket_c:socket) -> Package:
        bytes_package = socket_c.recv(ClientServerCommunicator.__MAX_BYTES)
        package = pickle.loads(bytes_package)
        #self._logger.info("Package read %s " % package.type)
        data = ""
        type = None
        if package.type == PackageType.INIT:
            #self._logger.info("Begin train read of %s" % package.payload)
            bytes_to_read = int(package.payload)
            to_read = ClientServerCommunicator.__MAX_BYTES
            while bytes_to_read > 0:
                if to_read > bytes_to_read:
                    to_read = bytes_to_read
                bytes_package_aux = b""
                bytes_package_aux = socket_c.recv(to_read)
                bytes_to_read -= len(bytes_package_aux)
                package_aux = pickle.loads(bytes_package_aux)
                data += package_aux.payload
                type = package_aux.type
                #self._logger.info("Package read %s, %s bytes remaining " % (package_aux.type, bytes_to_read))
            #self._logger.info("Read data complete")
            return Package(type, data)
        else:
            #self._logger.info("Read data complete")
            return package

    @staticmethod
    def send_data(socket_c:socket, package:Package):
        percent_deviation = 3
        bytes_serial_size = len(pickle.dumps(package))
        if bytes_serial_size >= ClientServerCommunicator.__MAX_BYTES:
            packages = []
            how_many_packages = bytes_serial_size // ClientServerCommunicator.__MAX_BYTES
            if bytes_serial_size // ClientServerCommunicator.__MAX_BYTES != 0:
                how_many_packages += 1
            deviation = percent_deviation * ClientServerCommunicator.__MAX_BYTES // 100
            max_package_size = ClientServerCommunicator.__MAX_BYTES - deviation
            payload_index = 0
            total_bytes = 0
            string_payload = str(package.payload)
            while payload_index < len(package.payload):
                new_package = Package(package.type, string_payload[payload_index:payload_index+max_package_size])
                new_bytes_package = pickle.dumps(Package)
                new_bytes_package_size = len(new_bytes_package)
                if new_bytes_package_size > ClientServerCommunicator.__MAX_BYTES:
                    max_package_size -= deviation
                else:
                    packages.append(new_package)
                    payload_index += max_package_size
                    total_bytes += new_bytes_package_size
            init_package = Package(PackageType.INIT,str(total_bytes))
            bytes_init_package = pickle.dumps(init_package)
            socket_c.sendall(bytes_init_package)
            for pack in packages:
                socket_c.sendall(pack)
        else:
            bytes_package = pickle.dumps(package)
            socket_c.sendall(bytes_package)

    def connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #self.__logger.info("Client socket created!")
        except socket.error:
            #self.__logger.error("Failed to create socket! Error :" + str(sys.exc_info()[1]))
            return -1

        try:
            # Connect to remote server
            #self.__logger.info("Initiate connection to %s:%s ..." % (self.__host, str(self.__port)))
            s.connect((self.__host, self.__port))
            #self.__logger.info("Connected to the server!")
            self.__server_socket = s
            return 0
        except:
            #self.__logger.error("Failed to connect to the server! Error :" + str(sys.exc_info()[1]))
            return -1

    def close_connection(self):
        if self.__server_socket is not None:
            self.__server_socket.close()

    def send_packs(self, package: Package):
        ClientServerCommunicator.send_data(self.__server_socket, package)