import datetime
import socket
import sys
from typing import Union
from communicators.abstract_communicator import AbstractCommunicator
from packages.abstract_package import AbstractPackage
from packages.client_package import *

from domain.package import Package as PackageDOM

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
    def __convert_bits_to_ip(bits):
        if len(bits) != 32:
            return ""
        ip_nr = []
        ip_nr.append(int("".join([str(el) for el in bits[0:8]]), 2))
        ip_nr.append(int("".join([str(el) for el in bits[8:16]]), 2))
        ip_nr.append(int("".join([str(el) for el in bits[16:24]]), 2))
        ip_nr.append(int("".join([str(el) for el in bits[24:32]]), 2))
        ip_str = ""
        for el in ip_nr:
            ip_str += str(el)+"."
        ip_str = ip_str[:-1] #remove the last .
        return ip_str

    @staticmethod
    def parse_raw_package(data:bytes):
        current_byte_index = 0
        current_bit_index = 0
        max_byte_index = 150
        data_bits = []
        for byte in data:
            for bit in range(7, -1, -1):
                if byte & (1 << bit):
                    data_bits.append(1)
                    #print('1', end='')
                else:
                    #print('0', end='')
                    data_bits.append(0)
                # current_bit_index += 1
            current_byte_index += 1
            if current_byte_index >= max_byte_index:
                break
        network_package = PackageDOM(None, 0, None, 0, None, None, None, None, 0, None)
        network_package.network_protocol = int("".join([str(el) for el in data_bits[0:4]]), 2)
        ipv4_h_len = int("".join([str(el) for el in data_bits[4:8]]), 2) * 4
        network_package.payload_size = int("".join([str(el) for el in data_bits[16:32]]), 2)
        network_package.transport_protocol = int("".join([str(el) for el in data_bits[72:80]]), 2)
        network_package.application_protocol = 0
        network_package.source_ip = RouterCommunicator.__convert_bits_to_ip(data_bits[96:128])
        network_package.destination_ip = RouterCommunicator.__convert_bits_to_ip(data_bits[128:160])

        t_proto_index = ipv4_h_len * 8  #8 bits
        payload_index = ipv4_h_len
        if network_package.transport_protocol == 6: # tcp
            network_package.source_port = int("".join([str(el) for el in data_bits[t_proto_index:t_proto_index + 16]]), 2)
            network_package.destination_port = int("".join([str(el) for el in data_bits[t_proto_index + 16:t_proto_index + 32]]), 2)
            tcp_h_len = int("".join([str(el) for el in data_bits[t_proto_index + 96:t_proto_index + 100]]), 2)
            network_package.payload_size -= tcp_h_len
            payload_index += tcp_h_len
        if network_package.transport_protocol == 17: # udp
            network_package.source_port = int("".join([str(el) for el in data_bits[t_proto_index:t_proto_index + 16]]), 2)
            network_package.destination_port = int("".join([str(el) for el in data_bits[t_proto_index + 16:t_proto_index + 32]]), 2)
            udp_h_len = 8 # always fixed header size
            network_package.payload_size -= udp_h_len
            payload_index += udp_h_len
        network_package.payload = data[payload_index:]
        network_package.arrival_time = datetime.datetime.now()
        return network_package

    @staticmethod
    def parse_header_package(data: bytes):
        print(data)
        # c0 a8 01 7b c7 e8 12 f8 d7 d3 00 00 01 bb 00 00 00 00 00 00 08
        network_package = PackageDOM("", 0, "", 0, 0, 0, 0, 0, 0, b'')
        ip_s = ""
        ip_d = ""
        port_s = 0
        port_d = 0
        data_len = 0
        network_proto = 0
        transport_proto = 0
        payload = b""
        index = 0
        for i in range(0, 4):
            network_package.source_ip += str(data[index + i]) + "."
        index += 4
        network_package.source_ip = network_package.source_ip[:-1]
        for i in range(0, 4):
            network_package.destination_ip += str(data[index + i]) + "."
        index += 4
        network_package.destination_ip = network_package.destination_ip[:-1]
        #print(data[index+0:index+4],int.from_bytes(data[index+0:index+2],"big"), str(data[index+3])+str(data[index+2])+str(data[index+1])+str(data[index + 0]))
        network_package.source_port = int.from_bytes(data[index+0:index+2],"big")
        index += 4
        #print(data[index + 0:index + 4], int.from_bytes(data[index+0:index+2],"big"), str(data[index+3])+str(data[index+2])+str(data[index+1])+str(data[index + 0]))
        network_package.destination_port = int.from_bytes(data[index+0:index+2],"big")
        index += 4
        #network_package.payload_size = int("0x"+str(data[index+3])+str(data[index+2])+str(data[index+1])+str(data[index + 0]),16)
        network_package.payload_size = int.from_bytes(data[index+0:index+4],"little")
        index += 4
        network_package.network_protocol = 4 # int(str(data[index+1])+str(data[index]),16)
        index += 2
        network_package.transport_protocol = data[index]
        index += 1
        network_package.payload = data[index:]
        network_package.application_protocol = 0
        network_package.arrival_time = datetime.datetime.now()
        return network_package

    @staticmethod
    def __read_package(data: bytes):
        type = data[0]
        if type == 0:
            return RouterCommunicator.parse_raw_package(data[1:])
        else:
            return RouterCommunicator.parse_header_package(data[1:])

    @staticmethod
    def __read_from_ext(socket_c: Union[socket,bytes]):
        if isinstance(socket_c,socket.SocketType):
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
            return RouterCommunicator.__read_package(socket_c)
        return []

    @staticmethod
    def __read_from_local(socket_c: socket):
        return []

    @staticmethod
    def read_data(socket_c: Union[socket,bytes]) -> Union[AbstractPackage, list]:
        #host, _ = socket_c.getpeername()
        #if host == "127.0.0.1":
        if isinstance(socket_c,socket.SocketType):
            return RouterCommunicator.__read_from_local(socket_c)
        else:
            return RouterCommunicator.__read_from_ext(socket_c)




#ceva cu socket.gethostname()

# if client_socket.gethostname() == "127.0.0.1":
#     self.__process_internal(data)
# else:
#     data = ClientServerCommunicator.read_data(client_socket)
#     self.__process_external(data)