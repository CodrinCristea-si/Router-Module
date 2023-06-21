from db.client_manager import DBClientManager
from db.test_manager import DBTestManagement
from db.sample_manager import DBSampleManager

from db.session import SessionMaker

from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from packages.infectivity_response import *
from logger.logger import Logger
from server.server import Server
from logger.logger import Logger

class InfectivityManager:
    def __init__(self,logger:Logger,session:SessionMaker = None):
        self.__session = session
        if session is None:
            self.__session = SessionMaker.create_scoped_session()
        self.__client_manager = DBClientManager(self.__session)
        self.__test_manager = DBTestManagement(self.__session)
        self.__sample_manager = DBSampleManager(self.__session)
        self.__logger = logger

    def new_client_connection(self,ip:str,mac:str):
        self.__client_manager.add_client(ip,mac)
        is_reachable = self.__check_if_client_can_be_reaced(ip)
        print("Client is reachable?", is_reachable)
        #self.__test_client(ip,mac)


    def remove_client_connection(self,ip:str,mac:str):
        self.__client_manager.remove_client(ip,mac)

    def get_all_clients(self):
        return self.__client_manager.get_all_clients()

    def __check_if_client_can_be_reaced(self,ip:str):
        com = ITC(Server._MAIN_IP,ITC._TESTER_PORT,self.__logger)
        com.connect()
        com.send_request(InfectivityRequest(InfectivityRequestType.CHECK_CLIENT,[ip]))
        response = com.read_response()
        is_reachable = False
        if response.type == InfectivityResponseType.STATUS_AVAILABLE:
            is_reachable = True
        return is_reachable

    def get_all_platforms(self):
        return self.__sample_manager.get_all_platforms()

    def get_all_categories(self):
        return self.__sample_manager.get_all_categories()

    def get_sample_stats(self):
        stat_plat = self.__sample_manager.get_samples_stats_by_platforms()
        stat_cat = self.__sample_manager.get_samples_stats_by_categories()
        return stat_plat, stat_cat
    
    def __test_client(self,ip:str,mac:str):
        self.__test_manager.add_test(ip,mac)
        self.__test_manager.begin_test(ip,mac)