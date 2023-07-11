

class Test:
    def __init__(self, client_ip, time_started,status,time_finished, id = 0):
        self.__id = id
        self.__client_ip = client_ip
        self.__time_started = time_started
        self.__status = status
        self.__time_finished = time_finished

    @property
    def id(self):
        return self.__id

    @id.setter
    def id(self, value):
        self.__id = value

    @property
    def client_ip(self):
        return self.__client_ip

    @property
    def time_started(self):
        return self.__time_started

    @property
    def status(self):
        return self.__status

    @property
    def time_finished(self):
        return self.__time_finished

