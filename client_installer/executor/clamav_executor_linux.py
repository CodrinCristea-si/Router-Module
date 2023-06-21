import copy
import json
import os.path
import os
import subprocess
import itertools
import pathlib

from executor.executor import *
from executor.results import ScanResults
from logger.logger import Logger


class ClamAVExecutorLinux(Executor):
    __CONFIG_FILE = "places_light.json"
    __INTERMEDIATE_RESULT_FILE = "temp.txt"
    __RESULT_FILE = "scan_results.txt"
    __INFECTED_REPORT_FILE = "infected.txt"

    __PLACES_LOCATION = "places"
    __TYPE_LOCATION = "type"
    __PATH_LOCATION = "path"
    __USER_REPLACEABLE = "$(username)"

    def __init__(self, logger: Logger):
        super().__init__(logger)

    def __get_all_home_users(self) -> list:
        self._logger.info("__get_all_home_users stared")
        cmd = ['ls', '/home']
        temp_pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        self._logger.info("Get Users process initiated")
        data = temp_pipe.communicate()
        users = []
        for el in data:
            if el is not None:
                users.append(str(el.decode("utf-8")).replace("\n", ""))
        # users = data.split(' \r\n')
        self._logger.info("Users identified : %s" % users)
        return users

    def __remove_replaceable_data(self, places: list) -> list:
        self._logger.info("__remove_replaceable_data stared")
        users = self.__get_all_home_users()
        self._logger.info("Users retrieved")
        paths = []
        print(places)
        for place in places:
            path = place[self.__PATH_LOCATION]
            if self.__USER_REPLACEABLE in path:
                place_to_modify = copy.deepcopy(place)
                path_to_modify = place_to_modify[self.__PATH_LOCATION]
                for user in users:
                    path_to_modify = path_to_modify.replace(self.__USER_REPLACEABLE, user)
                    print(path_to_modify)
                    place_to_modify[self.__PATH_LOCATION] = path_to_modify
                    if os.path.exists(path_to_modify) or os.path.islink(path_to_modify):
                        paths.append(place_to_modify)
            else:
                if os.path.exists(path) or os.path.islink(path):
                    paths.append(place)
        self._logger.info("Number of actual paths to scan %d" % (len(paths)))
        return paths

    def __get_most_common_places_to_scan(self) -> list:
        self._logger.info("__get_most_common_places_to_scan stared")
        data = {}
        current_dir = str(pathlib.Path(__file__).parent.resolve())
        with open(current_dir + "/" + self.__CONFIG_FILE, 'r') as json_file:
            data = json.load(json_file)
        self._logger.info("Configuration file loaded")
        places = data.get(self.__PLACES_LOCATION)
        self._logger.info("Got places to scan")
        if places is None:
            raise Exception("Config file missing!")
        places = self.__remove_replaceable_data(places)
        self._logger.info("Paths normalized")
        return places

    def __save_infected_file(self, file: str, mal_type: str):
        self._logger.info("Preparing to save infected file %s..." % file)
        current_dir = str(pathlib.Path(__file__).parent.resolve())
        with open(current_dir + "/" + self.__INFECTED_REPORT_FILE, 'a') as infected:
            infected.write(file + ' -> ' + mal_type + '\n')
        self._logger.info("infected file saved with success")

    def __clear_infected_history(self):
        self._logger.info("Preparing to delete infected history...")
        current_dir = str(pathlib.Path(__file__).parent.resolve())
        with open(current_dir + "/" + self.__INFECTED_REPORT_FILE, 'w') as infected:
            infected.write('')
        self._logger.info("History cleared with success")

    def __get_intermediate_results(self) -> tuple:
        self._logger.info("__get_intermediate_results started")
        malware = {}
        ratio = []
        current_dir = str(pathlib.Path(__file__).parent.resolve())
        with open(current_dir + "/" + self.__INTERMEDIATE_RESULT_FILE, 'r') as res:
            self._logger.info("Intermediate results opened")
            line = res.readline()
            while line is not None and line != "":
                print(line)
                if "FOUND" in line:
                    print("found")
                    # line format $path $malware_type FOUND
                    data = line.split(" ")
                    malware_type = data[-2]
                    if malware.get(malware_type) is None:
                        malware[malware_type] = 0
                    malware[malware_type] += 1
                    malware_path = "".join(list(itertools.chain(data[:-2])))
                    self._logger.info("Malware %s found" % malware_type)
                    self.__save_infected_file(malware_path, malware_type)
                    self._logger.info("Infected file %s saved" % malware_path)
                if "Scanned files" in line:
                    data = line.split(" ")
                    print(data)
                    files = int(data[-1].replace("\n", ""))
                    ratio.append(files)
                    self._logger.info("Intermediate scanned ratio found %s" % str(files))
                if "Infected files" in line:
                    data = line.split(" ")
                    print(data)
                    files = int(data[-1].replace("\n", ""))
                    ratio.append(files)
                    self._logger.info("Intermediate infected ratio found %s" % str(files))
                line = res.readline()
        self._logger.info("File successfully analised")
        print(malware, ratio)
        if ratio[0] > ratio[1]:
            ratio[0], ratio[1] = ratio[1], ratio[0]
            self._logger.info("Intermediate ratio reversed")
        os.remove(current_dir + "/" + self.__INTERMEDIATE_RESULT_FILE)
        self._logger.info("Intermediate results file removed")
        print(malware, ratio)
        return (malware, ratio)

    def __normalize_date_malware(self, malwares: dict):
        self._logger.info("__normalize_date_malware started")
        new_malwares = {}
        for malware in malwares:
            plain_name = copy.copy(malware)
            plain_name = plain_name.split("-")[0]
            new_malwares[plain_name] = malwares[malware]
        malwares = new_malwares
        self._logger.info("__normalize_date_malware done")

    def __initiate_scan(self, places: list):
        self._logger.info("__initiate_scan started")
        current_dir = str(pathlib.Path(__file__).parent.resolve())
        file_location = current_dir + "/" + self.__INTERMEDIATE_RESULT_FILE

        scan_cmd = 'clamscan -r $(path) -i --follow-dir-symlinks=2 --follow-file-symlinks=2 1>%s' % (file_location)
        # scan_file_cmd = 'clamscan $(path) -i --follow-file-symlinks=2'
        # scan_sym_cmd = 'clamscan $(path) -i --follow-dir-symlinks=2 --follow-file-symlinks=2'
        self.__clear_infected_history()
        self._logger.info("History cleared")
        malwares = {}
        ratios = [0, 0]
        for place in places:
            to_scan = copy.copy(scan_cmd).replace('$(path)', place[self.__PATH_LOCATION])
            print(to_scan.split(" "))
            # with open(file_location,'w') as f:
            #   f.write("")
            subprocess.run(['echo', '>', file_location])
            # subprocess.run(to_scan.split(' '))
            os.system(to_scan)
            self._logger.info("A scan process finished with success")
            malware, ratio = self.__get_intermediate_results()
            print(malware, ratio)
            self._logger.info("Intermidiate results retrieved with %s and ratio %s" % (str(len(malware)), str(ratio)))
            for mal in malware:
                print("muie")
                if malwares.get(mal) is None:
                    malwares[mal] = 0
                malwares[mal] += malware[mal]
            ratios[0] += ratio[0]
            ratios[1] += ratio[1]
            self._logger.info("Intermediate results counted")
        self.__normalize_date_malware(malwares)
        self._logger.info("Data normalized with success")
        return malwares, ratios

    def execute(self) -> ScanResults:
        self._logger.info("ClamAv Linux Executor begin execution... ")
        places_to_scan = self.__get_most_common_places_to_scan()
        self._logger.info("Places retrieved")
        malware, ratio = self.__initiate_scan(places_to_scan)
        self._logger.info(
            "Scan ran with success : number of malware %s with ration %s " % (str(len(malware)), str(ratio)))
        return ScanResults(malware, ratio)