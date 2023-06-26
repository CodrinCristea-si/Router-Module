
"""
Heuristics layout
heur.py --name  -> gives the name of the heuristics
heur.py --type  -> gives the type of the heuristics
heur.py --requirements   -> gives the list of resquest types

heur.py --load input    -> loads the input file to analyse
heur.py --execute output  -> runs the heuristics and returns the output in output file

heuristics types:
    ->static: check the test results from clients
    ->package: check the packages retrieved from router
    ->general: checks on any entity from db

requirement type
    CONNECTED_CLIENTS  -> json input fields [{ip,mac,type,score},...]
    LAST_MINUTE_PACKAGES -> json input fileds [{source_IP,source_port,destination_IP,
                            destination_port,network_protocol,transport_protocol,
                            application_protocol,arrived_at,payload},...]
    LAST_MINUTE_TESTS    -> json input format [{client_ip,client_mac,is_finished,results[{platform,category,name,score,times},...]},...]
    CLIENT_RESULTS
input format (json)
{
    response:[
        {
        "type":type1
        "input":[{...}]
        },
        {
        "type":type2
        "input":[{...}]
        },
        ...
    ]
}

output format (json)
{
    results: [
        {
          "ip":ip1,
          "mac":mac1,
          "score":score,
        },
        {
          "ip":ip2,
          "mac":mac2,
          "score":score,
        },
        ... or ...
        {
            "error":err
        }
    ]
}

check results request format
[
"ip":ip,
"mac":mac,
"results":results
]
"""
import json
import socket
import threading
import os
from datetime import datetime
import calendar
import subprocess
from server.server import Server
from logger.logger import Logger
from concurrent.futures import ThreadPoolExecutor
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from packages.infectivity_response import *
from enum import Enum
from domain.heuristic import Heuristic
from heuristics.data_snaps import DataSnapshots
import time
from orm.test_status import TestStatus
from orm.platform import Platform
from orm.category import Category
from orm.sample import Sample


class HeuristicType(Enum):
    STATIC = 1
    PACKAGE = 2
    GENERAL = 3


class RequirementType(Enum):
    CONNECTED_CLIENTS = 1
    LAST_MINUTE_PACKAGES = 2
    LAST_MINUTE_TESTS = 3
    CLIENT_RESULTS = 4
    LAST_MINUTE_HISTORY = 5
    PLATFORMS = 6
    CATEGORIES = 7
    SAMPLES = 8


class HeuristicServer(Server):
    __EURISTICS_STORAGE="heuristics/storage"

    def __init__(self, host: str, port: int, logger: Logger):
        super().__init__(host, port, logger)
        self._heur_list = set()
        self.__lock = threading.Lock()
        self.__max_workers = 3
        self.__thread_pool = ThreadPoolExecutor(max_workers=self.__max_workers)
        self._heur_list = self.__load_heuristics_from_device()
        #self.__load_heuristics_to_db(self._heur_list)
        self.__data_snapshot = DataSnapshots()
        #self.__thread_pool.submit(self.__check_for_malicious_behaviour)

    def __get_heur_type_by_str(self,type_str):
        ret_type = None
        try:
            type = int(type_str)
            for heur_type in HeuristicType:
                print(heur_type, heur_type.value, type)
                if heur_type.value == type:
                    ret_type = heur_type
                    break
        except Exception as e:
            print(e)
            ret_type = None
        return ret_type

    def __get_req_type_by_str(self,type_str):
        ret_type = None
        try:
            type = int(type_str)
            for req_type in RequirementType:
                if req_type.value == type:
                    ret_type = req_type
                    break
        except:
            ret_type = None
        return ret_type

    def __generate_file(self,basic_title:str):
        dt = datetime.now()
        utc_time = calendar.timegm(dt.utctimetuple())
        return str(basic_title) + str(utc_time)


    def __create_clients_result_req(self,ip:str,mac:str, results:list):
        req = {
            "type":RequirementType.CLIENT_RESULTS.value,
            "input":[{
                "ip":ip,
                "mac":mac,
                "results":results
            }]
        }
        return req

    def __create_last_minute_packages_req(self, packages:list):
        req = {
            "type":RequirementType.LAST_MINUTE_PACKAGES.value,
            "input":[]
        }
        for pack in packages:
            data_pack = {
                "id":pack.PackageID,
                "sip":pack.SourceIP,
                "sport":pack.SourcePort,
                "dip":pack.DestinationIP,
                "dport":pack.DestinationPort,
                "nproto":pack.NetworkProtocol,
                "tproto":pack.TransportProtocol,
                "aproto":pack.ApplicationProtocol,
                "arrive_time":pack.ArriveTime,
                "payload":pack.Payload
            }
            req.get("input").append(data_pack.copy())
        return req

    def __create_connected_clients_req(self, clients:list):
        req = {
            "type":RequirementType.CONNECTED_CLIENTS.value,
            "input":[]
        }
        for client in clients:
            data_client = {
                "id":client.ClientID,
                "ip":client.CurrentIP,
                "mac":client.SourcePort,
                "testing":client.IsTesting,
                "type":client.InfectivityType,
                "score":client.Score,
            }
            req.get("input").append(data_client.copy())
        return req


    def __convert_test_status_to_string(self,test_status):
        status_str = ""
        if test_status == TestStatus.RUNNING.value:
            status_str = "running"
        elif test_status == TestStatus.STARTED.value:
            status_str = "started"
        elif test_status == TestStatus.FINISHED.value:
            status_str = "finished"
        elif test_status == TestStatus.STOPPED.value:
            status_str = "stopped"
        return status_str

    def __create_last_minute_tests_req(self, tests:list):
        req = {
            "type":RequirementType.CONNECTED_CLIENTS.value,
            "input":[]
        }
        for test_set in tests:
            test = test_set[0]
            results = test_set[1]
            data_test = {
                "id":test.TestID,
                "client_ip":test.ClientID,
                "start_time":test.TimeTaken,
                "end_time":test.TimeFinished,
                "status":self.__convert_test_status_to_string(test.Status),
                "results":[],
            }
            for res in results:
                data_res = {
                    "sample_id":res.SampleID,
                    "times":res.NumberOfTimes
                }
                data_test.get("results").append(data_res.copy())
            req.get("input").append(data_test.copy())
        return req

    def __create_platforms_req(self, platforms:list):
        req = {
            "type":RequirementType.PLATFORMS.value,
            "input":[]
        }
        for plat in platforms:
            data_platform = {
                "id":plat.PlatformID,
                "name":plat.Name,
                "score":plat.Score
            }
            req.get("input").append(data_platform.copy())
        return req

    def __create_category_req(self, categories:list):
        req = {
            "type":RequirementType.CATEGORIES.value,
            "input":[]
        }
        for cat in categories:
            data_cat = {
                "id":cat.CategoryID,
                "name":cat.Name,
                "score":cat.Score
            }
            req.get("input").append(data_cat.copy())
        return req


    def __create_sample_req(self, samples:list):
        req = {
            "type":RequirementType.SAMPLES.value,
            "input":[]
        }
        for samp in samples:
            data_samp = {
                "id":samp.SampleID,
                "id_platform": samp.PlatformID,
                "id_category": samp.CategoryID,
                "name":samp.Name,
                "score":samp.Score
            }
            req.get("input").append(data_samp.copy())
        return req

    def __create_input_file(self,input_file:str,list_input_req:list):
        # input_file = self.__generate_file("input_static_")
        input_data ={"response": list_input_req}
        with open(input_file,"w") as file:
            json.dump(input_data,file,indent=4)
        # return input_file

    def __get_data_from_server(self,type,payload):
        comm = ITC("127.0.0.1", 5004, self._logger)
        comm.connect()
        pack = InfectivityRequest(type, payload)
        comm.send_request(pack)
        resp = comm.read_response()
        comm.close_connection()
        return resp.payload

    def __get_input_based_on_request(self, request, data = None):
        if request == RequirementType.CLIENT_RESULTS.value:
            return self.__create_clients_result_req(data[0],data[1],data[2])
        if request == RequirementType.LAST_MINUTE_PACKAGES.value:
            resp = self.__get_data_from_server(InfectivityRequestType.GET_LAST_MINUTE_PACKAGES,[])
            return self.__create_last_minute_packages_req(resp)
        if request == RequirementType.CONNECTED_CLIENTS.value:
            resp = self.__get_data_from_server(InfectivityRequestType.GET_CONNECTED_CLIENTS, [])
            return self.__create_connected_clients_req(resp)
        if request == RequirementType.LAST_MINUTE_TESTS.value:
            resp = self.__get_data_from_server(InfectivityRequestType.GET_LAST_MINUTE_TEST, [])
            return self.__create_last_minute_tests_req(resp)
        if request == RequirementType.PLATFORMS.value:
            resp = self.__get_data_from_server(InfectivityRequestType.GET_PLATFORMS, [])
            return self.__create_platforms_req(resp)
        if request == RequirementType.CATEGORIES.value:
            resp = self.__get_data_from_server(InfectivityRequestType.GET_CATEGORIES, [])
            return self.__create_category_req(resp)
        if request == RequirementType.SAMPLES.value:
            comm = ITC("127.0.0.1", 5004, self._logger)
            comm.connect()
            pack = InfectivityRequest(InfectivityRequestType.GET_SAMPLES, [])
            comm.send_request(pack)
            resp = comm.read_big_response()
            comm.close_connection()
            return self.__create_sample_req(resp.payload)
        return {}

    def __send_result_to_server(self, results):
        if results.get("results") is None:
            self._logger.error("Invalid results format type!")
        comm = ITC("127.0.0.1", 5004, self._logger)
        for el in results.get("results"):
            if el.get("error") is not None:
                self._logger.error("Error while analysing. Error: %s" % (el.get("error")))
            elif el.get("ip") is not None:
                pack = InfectivityResponse(InfectivityResponseType.HEURISTIC_RESULTS,[el])
                try:
                    comm.connect()
                    comm.send_response(pack)
                    comm.close_connection()
                except Exception as e:
                    self._logger.error("Error while sending to server! Error %s" % (e))

    def __send_analysis_results_to_server(self,analysis_results):
        for result in analysis_results:
            self.__send_result_to_server(result)

    def __execute_heuristic(self, heur:Heuristic, input_file,output_file):
        process = self.__run_heuristic(heur.path, ['--load', input_file, '--execute', output_file],wait_for_it=False)
        self._logger.info("heuristic loaded")
        process.wait()
        self._logger.info("heuristic executed")
        results = {}
        with open(output_file, "r") as file:
            results = json.load(file)
        self._logger.info("heuristic results retrieved")
        return results
        #self.__send_results_to_server(results)

    def __process_client_results(self, ip:str,mac:str, results:list, heur_list_copy:set):
        #load input
        path_storage = os.getcwd() + "\\" + HeuristicServer.__EURISTICS_STORAGE
        # input_file = self.__create_input_file_static(ip,mac,results)
        input_file = os.getcwd() + "\\" + self.__generate_file("input_static_")
        os.system(f"type nul> {input_file}")
        output_file = os.getcwd() + "\\" + self.__generate_file("output_static_")
        os.system(f"type nul> {output_file}")
        self._logger.info("input %s and output %s" %(input_file,output_file))
        self._logger.info("len heur %d" % (len(heur_list_copy)))
        static_analysis_results = []
        for heur in heur_list_copy:
            self._logger.info("is aci")
            #print(heur)
            if heur.type == HeuristicType.STATIC.value:
                self._logger.info("got heuristic %s" % (heur.name))
                list_input_req = []
                for req in heur.requirements:
                    input_req = self.__get_input_based_on_request(req,[ip,mac,results])
                    self._logger.info("got input of type %s and size %d" % (req, len(input_req.get("input"))))
                    list_input_req.append(input_req)
                self.__create_input_file(input_file,list_input_req)
                self._logger.info("input file created")
                results = self.__execute_heuristic(heur,input_file,output_file)
                self._logger.info("results %s"%(str(results)))
                static_analysis_results.append(results)
        #self._logger.warning(str(static_analysis_results))
        #self.__send_analysis_results_to_server(static_analysis_results)
        if os.path.exists(input_file):
            os.system(f"del {input_file}")
        if os.path.exists(output_file):
            os.system(f"del {output_file}")
        return static_analysis_results

    def __send_heuristic_to_db(self, heur:Heuristic):
        comm = ITC("127.0.0.1", 5004, self._logger)
        pack = InfectivityRequest(InfectivityRequestType.ADD_HEURISTIC, [heur])
        try:
            comm.connect()
            comm.send_request(pack)
            comm.close_connection()
        except Exception as e:
            self._logger.error("Error while sending to server! Error %s" % (e))

    def __load_heuristics_to_db(self, heur_list):
        for heur in heur_list:
            self.__send_heuristic_to_db(heur)

    def __run_heuristic(self,path:str, cmd:list, wait_for_it:bool = True):
        result = None
        command = []
        if ".py" in path:
            command = ['python.exe','%s' % (path)] + cmd
        else:
            command = ['%s' % (path)] + cmd
        if wait_for_it:
            result = subprocess.run(command, stdout=subprocess.PIPE)
        else:
            result = subprocess.Popen(command, stdout=subprocess.PIPE)
        return result

    def __load_heuristics_from_device(self):
        heur_list = set()
        path_storage = os.getcwd()+"\\"+ HeuristicServer.__EURISTICS_STORAGE
        if not os.path.exists(path_storage):
            os.mkdir(path_storage,0o755)
        list_files = os.listdir(path_storage)
        for file in list_files:
            #print("run", '%s\\%s'%(path_storage,file) )
            result = self.__run_heuristic('%s\\%s'%(path_storage,file) ,["--name"])
            name = result.stdout.decode("utf-8").strip()
            result = self.__run_heuristic('%s\\%s'%(path_storage,file), ["--type"])
            type = result.stdout.decode("utf-8").strip()
            result = self.__run_heuristic('%s\\%s'%(path_storage,file), ["--requirements"])
            requirements = str(result.stdout.decode("utf-8")).split(" ")
            #print(requirements, type, name, self.__get_heur_type_by_str(type))
            heur = Heuristic(file,'%s\\%s'%(path_storage,file),name,self.__get_heur_type_by_str(type).value,[self.__get_req_type_by_str(req).value for req in requirements if self.__get_req_type_by_str(req) is not None])
            #print(heur)
            heur_list.add(heur)
        return heur_list

    def __check_for_new_heuristics(self,old_list,new_list):
        set_difference = set(new_list) - set(old_list)
        return set_difference

    def __get_snap_data_based_on_req(self,request_type):
        if request_type == RequirementType.LAST_MINUTE_PACKAGES.value:
            return self.__data_snapshot.packages
        if request_type == RequirementType.LAST_MINUTE_TESTS.value:
            return self.__data_snapshot.tests
        if request_type == RequirementType.CONNECTED_CLIENTS.value:
            return self.__data_snapshot.connected_clients
        if request_type == RequirementType.LAST_MINUTE_HISTORY.value:
            return self.__data_snapshot.history
        return {}

    def __execute_heuristics_package(self, list_heuristics):
        input_file = os.getcwd() + "\\" + self.__generate_file("input_package_")
        os.system(f"type nul> {input_file}")
        output_file = os.getcwd() + "\\" + self.__generate_file("output_package_")
        os.system(f"type nul> {output_file}")
        analysis_results = []
        for heur in list_heuristics:
            if heur.type == HeuristicType.PACKAGE.value:
                list_input_req = []
                for req in heur.requirements:
                    input_req = self.__get_snap_data_based_on_req(req)
                    list_input_req.append(input_req)
                self.__create_input_file(input_file,list_input_req)
                results = self.__execute_heuristic(heur,input_file,output_file)
                analysis_results.append(results)
        self.__send_analysis_results_to_server(analysis_results)
        if os.path.exists(input_file):
            os.system(f"del {input_file}")
        if os.path.exists(output_file):
            os.system(f"del {output_file}")

    def __execute_heuristics_general(self, list_heuristics):
        input_file = os.getcwd() + "\\" + self.__generate_file("input_general_")
        os.system(f"type nul> {input_file}")
        output_file = os.getcwd() + "\\" + self.__generate_file("output_general_")
        os.system(f"type nul> {output_file}")
        analysis_results = []
        for heur in list_heuristics:
            if heur.type == HeuristicType.GENERAL.value:
                list_input_req = []
                for req in heur.requirements:
                    input_req = self.__get_snap_data_based_on_req(req)
                    list_input_req.append(input_req)
                self.__create_input_file(input_file,list_input_req)
                results = self.__execute_heuristic(heur,input_file,output_file)
                analysis_results.append(results)
        self.__send_analysis_results_to_server(analysis_results)
        if os.path.exists(input_file):
            os.system(f"del {input_file}")
        if os.path.exists(output_file):
            os.system(f"del {output_file}")

    def __load_data_snapshots(self):
        self.__data_snapshot.connected_clients = self.__get_input_based_on_request(RequirementType.CONNECTED_CLIENTS.value)
        self.__data_snapshot.packages = self.__get_input_based_on_request(RequirementType.LAST_MINUTE_PACKAGES.value)
        self.__data_snapshot.tests = self.__get_input_based_on_request(RequirementType.LAST_MINUTE_TESTS.value)
        self.__data_snapshot.history = self.__get_input_based_on_request(RequirementType.LAST_MINUTE_HISTORY.value)

    def __check_for_malicious_behaviour(self):
        while True:
            time.sleep(60)

            #check for new heuristics
            copy_heur_list = self.__load_heuristics_from_device()
            self.__lock.acquire()
            old_heur = self._heur_list.copy()
            self._heur_list = copy_heur_list.copy()
            self.__lock.release()
            differences = self.__check_for_new_heuristics(old_heur,copy_heur_list)
            self.__load_heuristics_to_db(differences)

            #create snapshots of data
            self.__load_data_snapshots()

            #execute heuristics
            self.__execute_heuristics_package(copy_heur_list)
            self.__execute_heuristics_general(copy_heur_list)

    def __process_request(self,client_socket:socket):
        try:
            package = ITC.read_request_socket(client_socket)
            print(package)
            if package is None:
                return
            if package.type == InfectivityRequestType.CHECK_RESULTS:
                self._logger.info("Check results client")
                ip, mac, results = package.payload
                self.__lock.acquire()
                self._logger.info("getting heuristics")
                copy_heur_list = self._heur_list.copy()
                self.__lock.release()
                self._logger.info("processing request")
                results = self.__process_client_results(ip, mac, results,copy_heur_list)
                pack = InfectivityResponse(InfectivityResponseType.HEURISTIC_RESULTS,results)
                ITC.send_data(client_socket,pack,self._logger)

        except Exception as e:
            self._logger.error("Error %s"%(e))

    def handle_request(self, client_socket: socket):
        self.__thread_pool.submit(self.__process_request,client_socket)