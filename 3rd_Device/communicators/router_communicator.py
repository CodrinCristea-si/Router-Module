import socket
import sys
from communicators.abstract_communicator import AbstractCommunicator
from packages.abstract_package import AbstractPackage
from packages.client_package import *

class RouterCommunicator(AbstractCommunicator):
    _MAX_BYTES = 1024

    @staticmethod
    def __read_client(types:int,data:bytes):
        ip = ""
        mac = []
        for i in range(1, 5):
            ip += str(data[i]) + "."
        ip = ip[:-1]
        for i in range(5, 11):
            base = str(hex(data[i]))[2:]
            mac += (base + ":")
        mac = "".join(mac[:-1])
        infec = data[11]
        print([types, ip, mac, infec])
        return [types, ip, mac, infec]

    @staticmethod
    def __read_package(types: int, data: bytes):
        ip_s = ""
        ip_d = ""
        port_s = 0
        port_d = 0
        data_len = 0
        network_proto = 0
        transport_proto = 0
        payload = b""
        index = 0
        for i in range(1, 5):
            ip_s += str(data[index + i]) + "."
            index += 1
        ip_s = ip_s[:-1]
        for i in range(1, 5):
            ip_d += str(data[index + i]) + "."
            index += 1
        ip_d = ip_d[:-1]
        port_s = data[index]
        index += 1
        port_d = data[index]
        index += 1
        data_len = data[index]
        index += 1
        network_proto = data[index]
        index += 1
        transport_proto = data[index]
        payload = data[index:]
        lista = [types, ip_s, ip_d, port_s, port_d, data_len, network_proto, transport_proto, payload]
        print(lista)
        return lista

    @staticmethod
    def __read_from_ext(socket_c: socket):
        data_size = int.from_bytes(socket_c.recv(RouterCommunicator._MAX_BYTES),"little")
        print(data_size)
        #pack = None
        if 0 < data_size:
            curr_poz = 0
            data = b""
            while curr_poz < data_size:
                payload = socket_c.recv(data_size-curr_poz)
                print(payload)
                curr_poz += len(payload)
                data += payload
            print(len(data))
            types = int(data[0])
            if types < 4:
                return RouterCommunicator.__read_client(types,data)
            else:
                return RouterCommunicator.__read_package(types, data)
        return []

    @staticmethod
    def __read_from_local(socket_c: socket):
        return []

    @staticmethod
    def read_data(socket_c: socket) -> AbstractPackage| []:
        host, _ = socket_c.getpeername()
        if host == "127.0.0.1":
            return RouterCommunicator.__read_from_local(socket_c)
        else:
            return RouterCommunicator.__read_from_ext(socket_c)




#ceva cu socket.gethostname()

# if client_socket.gethostname() == "127.0.0.1":
#     self.__process_internal(data)
# else:
#     data = ClientServerCommunicator.read_data(client_socket)
#     self.__process_external(data)