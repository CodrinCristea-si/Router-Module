

class Package:
    def __init__(self,source_ip, source_port, destination_ip, destination_port, n_proto,t_proto, a_proto,arrival_time, payload_size, payload):
        self.__source_ip = source_ip
        self.__source_port = source_port
        self.__destination_ip = destination_ip
        self.__destination_port = destination_port
        self.__n_proto = n_proto
        self.__t_proto = t_proto
        self.__a_proto = a_proto
        self.__arrival_time = arrival_time
        self.__payload_size = payload_size
        self.__payload = payload

    @property
    def source_ip(self):
        return self.__source_ip

    @source_ip.setter
    def source_ip(self, value):
        self.__source_ip = value

    @property
    def source_port(self):
        return self.__source_port

    @source_port.setter
    def source_port(self, value):
        self.__source_port = value

    @property
    def destination_ip(self):
        return self.__destination_ip

    @destination_ip.setter
    def destination_ip(self, value):
        self.__destination_ip = value

    @property
    def destination_port(self):
        return self.__destination_port

    @destination_port.setter
    def destination_port(self, value):
        self.__destination_port = value

    @property
    def network_protocol(self):
        return self.__n_proto

    @network_protocol.setter
    def network_protocol(self, value):
        self.__n_proto = value

    @property
    def transport_protocol(self):
        return self.__t_proto

    @transport_protocol.setter
    def transport_protocol(self, value):
        self.__t_proto = value

    @property
    def application_protocol(self):
        return self.__a_proto

    @application_protocol.setter
    def application_protocol(self, value):
        self.__a_proto = value

    @property
    def arrival_time(self):
        return self.__arrival_time

    @arrival_time.setter
    def arrival_time(self, value):
        self.__arrival_time = value

    @property
    def payload_size(self):
        return self.__payload_size

    @payload_size.setter
    def payload_size(self, value):
        self.__payload_size = value

    @property
    def payload(self):
        return self.__payload

    @payload.setter
    def payload(self, value):
        self.__payload = value

    def __str__(self):
        return f'%s:%s->%s:%s on (%d,%d,%d) of payload size %d at %s' \
            %(self.__source_ip,self.__source_port,self.__destination_ip,self.__destination_port, self.__n_proto, self.__t_proto, self.__a_proto, self.__payload_size, str(self.__arrival_time))
