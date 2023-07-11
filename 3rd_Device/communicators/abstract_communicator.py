import socket
from packages.abstract_package import AbstractPackage

class AbstractCommunicator:
    @staticmethod
    def send_data(socket_c: socket, package: AbstractPackage):
        raise NotImplementedError("This is an abstract method")

    @staticmethod
    def read_data(socket_c: socket) -> AbstractPackage:
        raise NotImplementedError("This is an abstract method")