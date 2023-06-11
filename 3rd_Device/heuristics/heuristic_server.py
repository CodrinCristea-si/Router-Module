
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
          "type":type1
        },
        {
          "ip":ip2,
          "mac":mac2,
          "type":type2
        },
        ... or ...
        {
            "error":err
        }
    ]
}

"""
import socket
import os
import subprocess
from server.server import Server
from logger.logger import Logger
from concurrent.futures import ThreadPoolExecutor
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator
from enum import Enum


class HeuristicType(Enum):
    STATIC = 1,
    PACKAGE = 2,
    GENERAL = 3


class RequirementType(Enum):
    CONNECTED_CLIENTS = 1,
    LAST_MINUTE_PACKAGES = 2,
    LAST_MINUTE_TESTS = 3


class HeuristicServer(Server):
    __EURISTICS_STORAGE="storage"

    def __init__(self, host: str, port: int, logger: Logger):
        super().__init__(host, port, logger)
        self._heur_list = []
        self.__max_workers = 2
        self.__thread_pool = ThreadPoolExecutor(max_workers=self.__max_workers)
        self.__load_heuristics()

    def __get_heur_type_by_str(self,type_str):
        type = int(type_str)
        ret_type = None
        for heur_type in HeuristicType:
            if heur_type.value == type:
                ret_type = heur_type
                break
        return ret_type

    def __get_req_type_by_str(self,type_str):
        type = int(type_str)
        ret_type = None
        for req_type in RequirementType:
            if req_type.value == type:
                ret_type = req_type
                break
        return ret_type

    def __load_heuristics(self):
        path_storage = os.getcwd()+"\\"+ HeuristicServer.__EURISTICS_STORAGE
        if os.path.exists(path_storage):
            os.mkdir(path_storage,755)
        list_files = os.listdir(path_storage)
        for file in list_files:
            result = subprocess.run(['%s\\%s'%(path_storage,file), '--name'], stdout=subprocess.PIPE)
            name = result.stdout
            result = subprocess.run(['%s\\%s' % (path_storage, file), '--type'], stdout=subprocess.PIPE)
            type = result.stdout
            result = subprocess.run(['%s\\%s' % (path_storage, file), '--requirements'], stdout=subprocess.PIPE)
            requirements = str(result.stdout).split(" ")
            heur = {
                "filename":file,
                "name":name,
                "type":self.__get_heur_type_by_str(type),
                "requirements": [self.__get_req_type_by_str(req) for req in requirements]
            }
            self._heur_list.append(heur)

    def __process_request(self,client_socket:socket):
        package = InfectivityTesterCommunicator.read_request_socket(client_socket)
        if package is None:
            return
        

    def handle_request(self, client_socket: socket):
        self.__thread_pool.submit(self.__process_request,client_socket)