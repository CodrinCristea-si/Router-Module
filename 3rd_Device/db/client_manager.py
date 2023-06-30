from orm.infectivity_type import InfectivityTypes
from db.session import SessionMaker
from db.db_manager import DBManager
from orm.client import Client

class DBClientManager(DBManager):
    def __init__(self, session:SessionMaker= None):
        #self.__session = None
        super().__init__(session)


    def add_client(self,ip:str,mac:str):
        # if self.__session is not None and self.__session.is_active:
        #     self.__session.close()
        # self.__session = SessionMaker.create_session()
        client = self.get_client_by_mac(mac)
        if client is None:
            client_new = Client(CurrentIP= ip, MAC=mac, IsConnected = 1, Score=0, IsTesting=0, InfectivityType=InfectivityTypes.DEFAULT.value)
            self._session.add(client_new)
        else:
            client.IsTesting=0
            client.CurrentIP = ip
            client.IsConnected = 1
            client.InfectivityType=InfectivityTypes.DEFAULT.value
        self._session.commit()
        # self._session.close()
        return 0

    def begin_client_test(self, ip:str,mac:str):
        # if self._session is not None and self._session.is_active:
        #     self._session.close()
        # self._session = SessionMaker.create_session()
        client = self.get_client(ip, mac)
        if client is None:
            raise Exception("There is not such user with ip %s and mac %s" % (ip, mac))
        if client.IsConnected == 0:
            raise Exception("User with ip %s and mac %s is not connected" % (ip, mac))
        if client.IsTesting == 1:
            raise Exception("User with ip %s and mac %s is being tested" % (ip, mac))
        client.IsTesting=1
        self._session.commit()
        # self._session.close()
        return 0

    def end_client_test(self, ip:str,mac:str):
        client = self.get_client(ip, mac)
        if client is None:
            raise Exception("There is not such user with ip %s and mac %s" % (ip, mac))
        elif client.IsConnected == 0:
            raise Exception("User with ip %s and mac %s is not connected" % (ip, mac))
        elif client.IsTesting == 0:
            raise Exception("User with ip %s and mac %s is not being tested" % (ip, mac))
        else:
            client.IsTesting=0
            self._session.commit()
        return 0

    def remove_client(self,ip:str,mac:str):
        client = self.get_client(ip,mac)
        if client is None:
            raise Exception("There is not such user with ip %s and mac %s" % (ip,mac))
        else:
            client.IsConnected = 0
            client.IsTesting = 0
        self._session.commit()
        return 0

    def transfer_client(self, ip:str,mac:str, type:int):
        client = self.get_client(ip, mac)
        if client is None:
            raise Exception("There is not such user with ip %s and mac %s" % (ip,mac))
        elif client.IsConnected == 0:
            raise Exception("User with ip %s and mac %s is not connected" % (ip, mac))
        elif type > 5 or type < 0:
            raise Exception("Unknown type")
        client.InfectivityType=type
        self._session.commit()
        # elif client.IsTesting:
        #     raise Exception("User with ip %s and mac %s is being tested" % (ip, mac))

    def get_client(self,ip:str,mac:str,type:int=None):
        client = None
        if type is None:
            client = self._session.query(Client).filter(Client.CurrentIP == ip,Client.MAC == mac).first()
        else:
            client = self._session.query(Client).filter(Client.CurrentIP == ip,Client.MAC == mac, Client.InfectivityType == type).first()
        return client

    @staticmethod
    def get_type_by_score(score):
        if score == 0:
            return 1 # uninfected
        if 0 < score < 20:
            return 3 # infected minor
        if 20 < score < 100:
            return 4 # infected major
        if score > 100:
            return 5 # infected sever

    @staticmethod
    def get_score_by_type(type):
        if type == 1:
            return 0  # uninfected
        if type == 2:
            return 0  # suspicious
        if type == 3:
            return 5  # infected minor
        if type == 4:
            return 20  # infected major
        if type == 5:
            return 100  # infected sever

    def set_client_score(self,ip:str,mac:str,score:int):
        client = self.get_client(ip, mac)
        if client is None:
            raise Exception("There is not such user with ip %s and mac %s" % (ip, mac))
        elif client.IsConnected == 0:
            raise Exception("User with ip %s and mac %s is not connected" % (ip, mac))
        elif score < 0:
            raise Exception("Negative score!")
        client.Score = score
        client.InfectivityType = DBClientManager.get_type_by_score(score)
        self._session.commit()

    def get_client_by_mac(self,mac:str):
        client = self._session.query(Client).filter(Client.MAC == mac).first()
        return client

    def get_client_by_id(self,id:int):
        client = self._session.query(Client).filter(Client.ClientID == int(id)).first()
        return client

    def get_connected_client_by_ip(self,ip:str):
        client = self._session.query(Client).filter(Client.CurrentIP == ip, Client.IsConnected == 1).first()
        return client

    def get_all_clients(self):
        clients = self._session.query(Client).all()
        return clients

    def get_all_connected_clients(self):
        ls_cl = self._session.query(Client).filter(Client.IsConnected == 1).all()
        return ls_cl