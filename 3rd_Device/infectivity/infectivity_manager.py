from db.client_manager import DBClientManager
from db.test_manager import DBTestManagement
from db.sample_manager import DBSampleManager
from db.packet_manager import DBPPackageManager
from db.heuristic_manager import DBHeuristicManagement
from orm.test_status import TestStatus
from orm.test import Test
from orm.client import Client
from orm.infectivity_type import InfectivityTypes

from domain.test import Test as TestDOM
from domain.sample import Sample as SampleDOM
from domain.heuristic import Heuristic as HeuristicDOM

from db.session import SessionMaker

from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from packages.infectivity_response import *
from logger.logger import Logger
from server.server import Server
from logger.logger import Logger

from domain.package import Package as PackDOM


from datetime import datetime
import calendar

class InfectivityManager:
    def __init__(self,logger:Logger,session:SessionMaker = None):
        self.__session = session
        if session is None:
            self.__session = SessionMaker.create_scoped_session()
        self.__client_manager = DBClientManager(self.__session)
        self.__test_manager = DBTestManagement(self.__session)
        self.__sample_manager = DBSampleManager(self.__session)
        self.__package_manager = DBPPackageManager(self.__session)
        self.__heuristics_manager = DBHeuristicManagement(self.__session)
        self.__logger = logger

    def __convert_date_to_unix(self, date_str):
        input_date_time_str = date_str
        input_date_time = datetime.strptime(input_date_time_str, '%Y-%m-%d %H:%M:%S')

        unix_time = input_date_time.timestamp()
        return unix_time

    def new_client_connection(self,ip:str,mac:str):
        self.__client_manager.add_client(ip,mac)
        is_reachable = self.check_if_client_can_be_reached(ip)
        print("Client is reachable?", is_reachable)
        if is_reachable:
            client = self.__client_manager.get_client(ip,mac)
            test = self.__test_manager.get_last_test_for_client(ip,mac)

            # if it does not have a test done in less than 12 hours then test him
            if not (test.Status == TestStatus.RUNNING.value or test.Status == TestStatus.STOPPED.value \
                or (test.Status == TestStatus.FINISHED.value \
                and self.__convert_date_to_unix(str(datetime.now())) - self.__convert_date_to_unix(str(test.TimeFinished)) < 43200)): #12 hours
                self.scan_client(ip,mac)
        #self.__test_client(ip,mac)

    def get_connected_clients(self):
        ls_cl = self.__client_manager.get_all_connected_clients()
        return ls_cl

    def remove_client_connection(self,ip:str,mac:str):
        self.__client_manager.remove_client(ip,mac)

    def get_all_clients(self):
        return self.__client_manager.get_all_clients()

    def check_if_client_can_be_reached(self,ip:str):
        com = ITC(Server._MAIN_IP,ITC._TESTER_PORT,self.__logger)
        com.connect()
        com.send_request(InfectivityRequest(InfectivityRequestType.CHECK_CLIENT,[ip]))
        response = com.read_response()
        com.close_connection()
        is_reachable = False
        if response.type == InfectivityResponseType.STATUS_AVAILABLE:
            is_reachable = True
        return is_reachable

    def scan_client(self, ip:str, mac:str):
        self.__test_manager.add_test(ip, mac)
        self.__client_manager.begin_client_test(ip,mac)
        self.__test_manager.begin_test(ip,mac)
        com = ITC(Server._MAIN_IP, ITC._TESTER_PORT, self.__logger)
        com.connect()
        com.send_request(InfectivityRequest(InfectivityRequestType.SCAN_CLIENT, [ip]))
        com.close_connection()

    def scan_client_by_ip(self, ip:str):
        client = self.__client_manager.get_connected_client_by_ip(ip)
        self.scan_client(client.CurrentIP,client.MAC)

    def check_heuristic_results_interpretation(self, results):
        #{'results': [{'name': 'Interpret Client Scan Results', 'ip': '192.168.1.1', 'mac': '00:11:22:33:44:55', 'score': 150}]}
        if results.get("results") is None:
            raise Exception("Invalid results format!")
        #centralisex results
        clients_states = {}
        for res in results.get("results"):
            #client = self.__client_manager.get_client(res.get("ip"), res.get("mac"))
            client_details = (res.get("ip"), res.get("mac"))
            if client_details not in clients_states:
                clients_states[client_details] = {
                    "score":-1,
                    "dominant_type":-1,
                    "must_be_scanned":None
                }
            if res.get("score") is not None:
                clients_states[client_details]["score"] += int(res.get("score"))
            elif res.get("type") is not None:
                if int(res.get("type")) > clients_states[client_details]["dominant_type"]:
                    clients_states[client_details]["dominant_type"] = int(res.get("type"))
                if int(res.get("type")) == InfectivityTypes.SUSPICIOUS.value:
                    clients_states[client_details]["must_be_scanned"] = True
        for cl_res in clients_states:
            if clients_states[cl_res].get("score") > 0:
                self.__client_manager.set_client_score(cl_res[0], cl_res[1], clients_states[cl_res].get("score"))
                cl = self.__client_manager.get_client(cl_res[0], cl_res[1])
                self.transfer_client(cl_res[0], cl_res[1],cl.InfectivityType, should_propagate=True)
            if clients_states[cl_res].get("dominant_type") > 0:
                self.transfer_client(cl_res[0], cl_res[1], clients_states[cl_res].get("dominant_type"), should_propagate=True)
            if clients_states[cl_res].get("must_be_scanned") is not None and clients_states[cl_res].get("must_be_scanned"):
                self.scan_client(cl_res[0], cl_res[1])

    def __save_test_results(self,client,results):
        for malware in results:
            platform, category, name = DBSampleManager.parse_malware_name(malware)
            print(platform, category, name)
            count = results.get(malware)
            for i in range(count):
                self.__test_manager.add_test_results(client.CurrentIP, client.MAC, [(platform, category, name)])

    def __convert_infect_type_nr_to_infect_type_status(self,infec_type):
        infec_status = None
        for el in InfectivityTypes:
            if el.value == infec_type:
                infec_status = el
                break
        return infec_status

    def transfer_client_generic(self,ip:str,mac:str,score:int,infect_type:int):
        client = self.__client_manager.get_client(ip,mac)
        if client.Score != score:
            self.__client_manager.set_client_score(ip,mac,score)
        if client.InfectivityType != infect_type:
            self.transfer_client(ip,mac,infect_type,True)
        return True


    def add_test_results(self, results):
        ip, test_results = results[0], results[1]
        client = self.__client_manager.get_connected_client_by_ip(ip)
        self.__save_test_results(client,test_results.get("malware"))
        self.__test_manager.finish_test(client.CurrentIP, client.MAC)
        self.__client_manager.end_client_test(client.CurrentIP,client.MAC)
        req = InfectivityRequest(InfectivityRequestType.CHECK_RESULTS,[client.CurrentIP,client.MAC,test_results])
        comm = ITC("127.0.0.1", 5003, self.__logger)
        comm.connect()
        comm.send_request(req)
        resp = comm.read_response()
        comm.close_connection()
        self.check_heuristic_results_interpretation(resp.payload[0])

    def update_router(self,req:InfectivityRequest):
        comm = ITC("192.168.1.2", 5005, self.__logger)
        comm.connect()
        comm.send_request(req)
        comm.close_connection()

    def transfer_client(self,ip,mac,state, should_propagate=False):
        self.__client_manager.transfer_client(ip,mac, state)
        if should_propagate:
            req = InfectivityRequest(InfectivityRequestType.TRANSFER_CLIENT,[ip,mac, state])
            self.update_router(req)

    def get_all_platforms(self):
        return self.__sample_manager.get_all_platforms()

    def get_all_categories(self):
        return self.__sample_manager.get_all_categories()

    def get_sample_stats(self):
        stat_plat = self.__sample_manager.get_samples_stats_by_platforms()
        stat_cat = self.__sample_manager.get_samples_stats_by_categories()
        return stat_plat, stat_cat

    def add_package(self, network_pack:PackDOM):
        self.__package_manager.add_packager(network_pack.source_ip, network_pack.source_port,network_pack.destination_ip,network_pack.destination_port,network_pack.network_protocol, network_pack.transport_protocol, network_pack.application_protocol,network_pack.payload_size,network_pack.payload)

    def get_all_samples(self):
        return self.__sample_manager.get_all_samples()

    def __test_client(self,ip:str,mac:str):
        self.__test_manager.add_test(ip,mac)
        self.__test_manager.begin_test(ip,mac)
        # inca ceva


    def __convert_status_to_test_status(self, test_status):
        stats = None
        for el in TestStatus:
            if el.value == test_status:
                stats = el
                break
        return stats

    def get_all_running_tests(self):
        ls_test_dom = []
        ls_test_orm = self.__test_manager.get_all_running_tests()
        for test in ls_test_orm:
            client= self.__client_manager.get_client_by_id(test.ClientID)
            stats = self.__convert_status_to_test_status(test.Status)
            test = TestDOM(client.CurrentIP,test.TimeTaken,TestStatus.RUNNING if stats is None else stats, test.TimeFinished, test.TestID)
            ls_test_dom.append(test)
        return ls_test_dom

    def get_tests_for_client(self,ip:str,mac:str):
        ls_test_dom = []
        client = self.__client_manager.get_client(ip,mac)
        ls_test = self.__test_manager.get_tests_for_client_id(client.ClientID)
        for test in ls_test:
            stats = self.__convert_status_to_test_status(test.Status)
            test = TestDOM(client.CurrentIP, test.TimeTaken, TestStatus.STOPPED if stats is None else stats,
                           test.TimeFinished, test.TestID)
            ls_test_dom.append(test)
        return ls_test_dom

    # def get_finished_tests_for_client(self,ip:str,mac:str):
    #     return

    def get_nr_tests_per_day_for_ip(self,ip:str,mac:str):
        client = self.__client_manager.get_client(ip, mac)
        ls_test = self.__test_manager.get_tests_for_client_id(client.ClientID)
        tests_per_day = {}
        for test in ls_test:
            time_str = test.TimeTaken.strftime("%Y-%m-%d")
            if time_str not in tests_per_day.keys():
                tests_per_day[time_str] = 0
            tests_per_day[time_str] += 1
        return tests_per_day

    def get_nr_malware_per_day_for_ip(self,ip:str,mac:str):
        client = self.__client_manager.get_client(ip, mac)
        ls_test = self.__test_manager.get_tests_for_client_id(client.ClientID)
        malware_per_day = {}
        for test in ls_test:
            time_str = test.TimeTaken.strftime("%Y-%m-%d")
            if time_str not in malware_per_day.keys():
                malware_per_day[time_str] = 0
            test_res = self.__test_manager.get_test_results_for_test(test.TestID)
            malware_per_day[time_str] += len(test_res)
        return malware_per_day

    def get_sample_data_by_id(self,sample_id):
        sample_db = self.__sample_manager.get_sample_by_id(sample_id)
        platform_db = self.__sample_manager.get_platform_by_id(sample_db.PlatformID)
        category_db = self.__sample_manager.get_category_by_id(sample_db.CategoryID)
        sample_dom = SampleDOM(platform_db.Name,category_db.Name,sample_db.Name)
        return sample_dom

    def get_test_by_id(self,test_id):
        test = self.__test_manager.get_test_by_id(test_id)
        print("test id", test.TestID, test.ClientID)
        client = self.__client_manager.get_client_by_id(test.ClientID)
        test_dom = TestDOM(client.CurrentIP, test.TimeTaken, self.__convert_status_to_test_status(test.Status),test.TimeFinished, test.TestID)
        test_res = self.__test_manager.get_test_results_for_test(test.TestID)
        malwares = {}
        for res in test_res:
            sample = self.get_sample_data_by_id(res.SampleID)
            sample_name = sample.platform+"."+sample.category+"."+sample.name
            if sample_name not in malwares.keys():
                malwares[sample_name]=0
            malwares[sample_name] += 1

        return test_dom, malwares

    def get_last_minutes_packages(self,ip:str,nr_min:int):
        ls_packs = self.__package_manager.get_last_minutes_packages(ip,nr_min)
        return ls_packs

    def get_t_proto_nr_packages_from_last_minute(self, ip:str, nr_min:int):
        ls_packs = self.get_last_minutes_packages(ip,nr_min)
        ls_stats = {}
        for pack in ls_packs:
            str_t_proto = str(pack.TransportProtocol)
            if str_t_proto not in ls_stats.keys():
                ls_stats[str_t_proto] = 0
            ls_stats[str_t_proto] += 1
        return ls_stats

    def get_last_nr_packages(self,ip:str,nr_packs:int):
        ls_packs = self.__package_manager.get_last_nr_packages(ip,nr_packs)
        return ls_packs

    def get_pack_by_id(self,pack_id:int):
        pack = self.__package_manager.get_package(pack_id)
        return pack

    def get_all_heuristics(self):
        ls_heur = self.__heuristics_manager.get_all_heuristics()
        return ls_heur

    def add_heuristic(self,heur:HeuristicDOM):
        self.__heuristics_manager.add_heuristics_dom(heur)
