import sys
import pickle
import socket

from logger.logger import Logger
from server.server import Server
from concurrent.futures import ThreadPoolExecutor
from infectivity.infectivity_manager import InfectivityManager
from packages.infectivity_request import *
from packages.infectivity_response import *
from db.session import SessionMaker
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator

class InfectivityServer(Server):

    def __init__(self,ip,port,logger:Logger):
        super().__init__(ip,port,logger)
        self.__max_workers = 8
        self.__thread_pool = ThreadPoolExecutor(max_workers=self.__max_workers)
        self.__is_automatic_lockdown = True
        self.__is_lockdown = False
        #super().run_server()

    def __add_task_executor(self,client_socket:socket):
        self._logger.info("__add_task_executor")
        session = None
        try:
            session = SessionMaker.create_scoped_session()
            self._logger.info("Session created %s" %(session))
            infectivity_manager = InfectivityManager(self._logger,session)
            self._logger.info("manager created")
            package = InfectivityTesterCommunicator.read_request_socket(client_socket)
            if package is None:
                return
            # prevent infinite loops for transfer, lockdown automatic between router and server
            if isinstance(package,InfectivityRequest):
                self._logger.info("Payload of type %s" % (package.type))
                if package.type == InfectivityRequestType.ADD_CLIENT:
                    self._logger.info("add client")
                    ip, mac = package.payload
                    infectivity_manager.new_client_connection(ip,mac)
                if package.type == InfectivityRequestType.REMOVE_CLIENT:
                    self._logger.info("remove client")
                    ip,mac = package.payload
                    infectivity_manager.remove_client_connection(ip,mac)
                if package.type == InfectivityRequestType.GET_PLATFORMS:
                    self._logger.info("get all platforms")
                    plats = infectivity_manager.get_all_platforms()
                    print(plats)
                    response = InfectivityResponse(InfectivityResponseType.PLATFORMS,plats)
                    InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
                if package.type == InfectivityRequestType.GET_CATEGORIES:
                    self._logger.info("get all categories")
                    cats = infectivity_manager.get_all_categories()
                    print(cats)
                    response = InfectivityResponse(InfectivityResponseType.CATEGORIES,cats)
                    InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
                if package.type == InfectivityRequestType.GET_SAMPLE_STATS:
                    self._logger.info("get sample stats")
                    stats = infectivity_manager.get_sample_stats()
                    print(stats)
                    response = InfectivityResponse(InfectivityResponseType.SAMPLE_STATS,stats)
                    InfectivityTesterCommunicator.send_data(client_socket,response,self._logger)
                if package.type == InfectivityRequestType.GET_ALL_CLIENTS:
                    self._logger.info("get all clients")
                    clients = infectivity_manager.get_all_clients()
                    response = InfectivityResponse(InfectivityResponseType.ALL_CLIENTS, clients)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.ADD_PACKAGE:
                    self._logger.info("add package")
                    network_pack = package.payload[0]
                    infectivity_manager.add_package(network_pack)
                if package.type == InfectivityRequestType.GET_SAMPLES:
                    self._logger.info("get all samples")
                    samples = infectivity_manager.get_all_samples()
                    response = InfectivityResponse(InfectivityResponseType.SAMPLES,samples)
                    InfectivityTesterCommunicator.send_big_data(client_socket,response,self._logger)
                if package.type == InfectivityRequestType.GET_ALL_RUNNING_TESTS:
                    self._logger.info("get all running tests")
                    ls_tests = infectivity_manager.get_all_running_tests()
                    response = InfectivityResponse(InfectivityResponseType.RUNNING_TESTS, ls_tests)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_NETWORK_CONFIGURATION:
                    self._logger.info("get network configs")
                    response = InfectivityResponse(InfectivityResponseType.NETWORK_CONFIGS, [{"automatic":self.__is_automatic_lockdown,"lockdown":self.__is_lockdown}])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_CONNECTED_CLIENTS:
                    self._logger.info("get connected clients")
                    ls_cl = infectivity_manager.get_connected_clients()
                    response = InfectivityResponse(InfectivityResponseType.CONNECTED_CLIENTS, ls_cl)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.LOCKDOWN_SETTINGS:
                    self._logger.info("set lockdown")
                    self.__is_lockdown = True if package.payload[0] == True else False
                    host, _ = client_socket.getpeername()
                    if host == "127.0.0.1": # ui
                        req = InfectivityRequest(InfectivityRequestType.LOCKDOWN_SETTINGS,[self.__is_lockdown])
                        infectivity_manager.update_router(req)
                if package.type == InfectivityRequestType.AUTOMATIC_SETTINGS:
                    self._logger.info("set automatic")
                    self.__is_automatic_lockdown = True if package.payload[0] == True else False
                    host, _ = client_socket.getpeername()
                    if host == "127.0.0.1": # ui
                        req = InfectivityRequest(InfectivityRequestType.AUTOMATIC_SETTINGS,[self.__is_automatic_lockdown])
                        infectivity_manager.update_router(req)
                if package.type == InfectivityRequestType.REACH_CLIENT:
                    self._logger.info("reach client")
                    ip = package.payload[0]
                    is_reachable = True
                    try:
                        is_reachable = infectivity_manager.check_if_client_can_be_reached(ip)
                    except Exception as e:
                        self._logger.error("Failed to reach client! Error %s" % (e))
                        is_reachable = False
                    response = InfectivityResponse(InfectivityResponseType.REACH_RESPONSE, [is_reachable])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.SCAN_CLIENT:
                    self._logger.info("scan client")
                    ip = package.payload[0]
                    is_scanned = True
                    try:
                        infectivity_manager.scan_client_by_ip(ip)
                        is_scanned = True
                    except Exception as e:
                        self._logger.error("Failed to begin the client scan! Error %s" %(e))
                        is_scanned = False
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, [is_scanned])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.TRANSFER_CLIENT_EXTENDED:
                    self._logger.info("transfer generic client")
                    ip,mac, score, infec_type = package.payload[0], package.payload[1], int(package.payload[2]), int(package.payload[3])
                    print(package)
                    is_transfered = True
                    try:
                        is_transfered = infectivity_manager.transfer_client_generic(ip,mac,score,infec_type)
                    except Exception as e:
                        self._logger.error("Failed to transfer client %s! Error %s" %(ip,e))
                        is_transfered = False
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, [is_transfered])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.TRANSFER_CLIENT:
                    self._logger.info("transfer client")
                    ip,mac, infec_type = package.payload[0], package.payload[1], package.payload[2]
                    infectivity_manager.transfer_client(ip,mac,infec_type,False)
                if package.type == InfectivityRequestType.GET_TESTS_FOR_CLIENT:
                    self._logger.info("get tests for client")
                    ip,mac = package.payload[0], package.payload[1]
                    ls_tests = infectivity_manager.get_tests_for_client(ip,mac)
                    response = InfectivityResponse(InfectivityResponseType.CLIENT_TESTS, ls_tests)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_TESTS_FOR_CLIENT_PER_DAY:
                    self._logger.info("get tests per day for client")
                    ip,mac = package.payload[0], package.payload[1]
                    ls_tests = infectivity_manager.get_nr_tests_per_day_for_ip(ip,mac)
                    response = InfectivityResponse(InfectivityResponseType.CLIENT_TESTS, ls_tests)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_MALWARE_FOR_CLIENT_PER_DAY:
                    self._logger.info("get malware per day for client")
                    ip,mac = package.payload[0], package.payload[1]
                    ls_tests = infectivity_manager.get_nr_malware_per_day_for_ip(ip,mac)
                    response = InfectivityResponse(InfectivityResponseType.CLIENT_TESTS, ls_tests)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_TEST:
                    self._logger.info("get tests details")
                    print(package)
                    test_id = package.payload[0]
                    test_det, malware_det = infectivity_manager.get_test_by_id(test_id)
                    response = InfectivityResponse(InfectivityResponseType.TEST_DETAILS, [test_det, malware_det])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_PACKAGES_PROTO_STATS:
                    self._logger.info("get package t proto stats")
                    ip,nr = package.payload[0], int(package.payload[1])
                    ls_packs = infectivity_manager.get_t_proto_nr_packages_from_last_minute(ip,nr)
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, ls_packs)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_LAST_NR_PACKAGES:
                    self._logger.info("get package last nr")
                    ip,nr = package.payload[0], int(package.payload[1])
                    ls_packs = infectivity_manager.get_last_nr_packages(ip,nr)
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, ls_packs)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_PACKAGE_PAYLOAD:
                    self._logger.info("get package payload")
                    id_pack = int(package.payload[0])
                    pack = infectivity_manager.get_pack_by_id(id_pack)
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, [pack])
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.GET_ALL_HEURISTICS:
                    self._logger.info("get all heuristic")
                    ls_heur = infectivity_manager.get_all_heuristics()
                    response = InfectivityResponse(InfectivityResponseType.GENERIC_RESPONSE, ls_heur)
                    InfectivityTesterCommunicator.send_data(client_socket, response, self._logger)
                if package.type == InfectivityRequestType.ADD_HEURISTIC:
                    self._logger.info("get add heuristic")
                    heur = package.payload[0]
                    infectivity_manager.add_heuristic(heur)
                if package.type == InfectivityRequestType.ARE_YOU_AWAKE:
                    pack = InfectivityResponse(InfectivityResponseType.I_AM_AWAKE, [])
                    InfectivityTesterCommunicator.send_data(client_socket, pack, self._logger)
                    client_socket.close()
            elif isinstance(package,InfectivityResponse):
                if package.type == InfectivityResponseType.TEST_RESULTS:
                    self._logger.info("test results")
                    infectivity_manager.add_test_results(package.payload)
            client_socket.close()
        #check after 12 hours that all clients tests are finished and clients are still reachable
        except Exception as e:
            self._logger.error("Error: %s" %(e))
        if session is not None:
            session.remove()

    def handle_request(self,client_socket:socket):
        self.__thread_pool.submit(self.__add_task_executor,client_socket)






