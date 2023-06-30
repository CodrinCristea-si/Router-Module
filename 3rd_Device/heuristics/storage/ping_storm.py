
"""
{type:PackageType.RESULTS, payload:{'malware': {'Win.Trojan.EmbeddedDotNetBinary-9940868-0': 2}, 'ratio': [2, 14213]}}
"""

import sys
import optparse
import json
import os
from datetime import datetime
import calendar


class ICMPStormDetector:
    NAME = "ICMP Storm Single Client Detection Heuristic"
    TYPE = 2
    REQUIREMENTS = [1, 2]

    def __init__(self):
        self.__client_results = []
        self.__requirements = {}
        self.__errors = []
        self.__can_operate = False


    @staticmethod
    def show_name(option, opt_str, value, parser):
        print(ICMPStormDetector.NAME)

    @staticmethod
    def show_type(option, opt_str, value, parser):
        print(ICMPStormDetector.TYPE)

    @staticmethod
    def show_requirements(option, opt_str, value, parser):
        print("".join([str(el)+" " for el in ICMPStormDetector.REQUIREMENTS]))

    def __parse_input_file(self):
        self.__errors = []
        if len(self.__requirements) == 0:
            self.__errors.append("No input data found!")
            return False
        if self.__requirements.get("response") is None:
            self.__errors.append("No response field found!")
            return False
        if len(self.__requirements.get("response")) <= 0:
            self.__errors.append("No response data found!")
            return False
        for req in ICMPStormDetector.REQUIREMENTS:
            is_req_satisfied = False
            for req_input in self.__requirements.get("response"):
                if req_input.get("type") is not None and req_input.get("type") == req and len(req_input.get("input")) >= 0:
                    is_req_satisfied = True
            if not is_req_satisfied:
                self.__errors.append("Requirement %d not satisfied!" % (req))
        if len(self.__errors) != 0:
            return False
        return True


    def load(self,option, opt_str, value, parser):
        self.__can_operate = False
        input_file = value
        #print(input_file)
        with open(input_file,"r") as file:
            self.__requirements = json.load(file)
        is_ok = self.__parse_input_file()
        if not is_ok:
            self.__errors.append("Invalid or incomplete input file")
            #self.__save_load_file(self.__errors)
            return False
        self.__can_operate = True
        #print(self.__requirements)
        return True

    def __get_list_according_to_req(self,req_type):
        list_to_return = []
        for req_input in self.__requirements.get("response"):
            if req_input.get("type") is not None and req_input.get("type") == req_type:
                list_to_return = req_input.get("input")
        return list_to_return

    def __save_results_file(self,output_file, results_list):
        results_final = {
            "results": results_list
        }
        with open(output_file,"w") as file:
            json.dump(results_final, file)

    def __get_mac_by_ip(self, ip):
        mac = ""
        connected_clients = self.__get_list_according_to_req(1)
        for client in connected_clients:
            if client.get("ip") == ip:
                mac = client.get("mac")
                break
        return mac

    def __convert_date_to_unix(self, date_str):
        input_date_time_str = date_str
        input_date_time = datetime.strptime(input_date_time_str, '%Y-%m-%d %H:%M:%S')
        unix_time = input_date_time.timestamp()
        return unix_time

    def __check_for_single_ping_storm(self,list_packages:list):
        is_icmp_storm = False
        if list_packages is not None and len(list_packages) > 0:
            max_per_second = 2
            threshold_sec= 5
            max_for_threshold_seconds = max_per_second * threshold_sec
            #print(list_packages[0])
            current_second = self.__convert_date_to_unix(list_packages[0].get("arrive_time"))
            current_packs_storm = 0
            icmp_per_sec = []
            # get icmps per second
            for pack in list_packages:
                arrive_sec = self.__convert_date_to_unix(pack.get("arrive_time"))
                if arrive_sec != current_second:
                    current_second = arrive_sec
                    icmp_per_sec.append(current_packs_storm)
                    current_packs_storm = 0
                if pack.get("tproto") == 1:
                    current_packs_storm += 1
            icmp_per_sec.append(current_packs_storm)
            #print("icmp_per_sec",icmp_per_sec)

            #get icmps per max max_for_threshold_seconds seconds
            max_per_threshold_sec_icmp = []
            for i in range(0,len(icmp_per_sec) - threshold_sec):
                max_per_threshold_sec_icmp.append(sum(icmp_per_sec[i:i+threshold_sec]))
            #print("max_per_threshold_sec_icmp",max_per_threshold_sec_icmp)

            #check if there are more icmps than usual
            for el in max_per_threshold_sec_icmp:
                if el > max_for_threshold_seconds:
                    is_icmp_storm = True
                    break
        # else:
        #     print("nasol")
        return is_icmp_storm

    def __analyse_input(self):
        score_for_found_guilty = 10
        type_for_guilty = 2
        packages = self.__get_list_according_to_req(2)
        connected_clients  = self.__get_list_according_to_req(1)
        client_dict = {}
        analysis_results = []
        #print("incepem")
        # make a client dictionary of type client_ip:[list_of_package_comming_from_that_ip]
        for client in connected_clients:
            if client.get("ip") not in client_dict.keys():
                client_dict[client.get("ip")] = []
        for pack in packages:
            #print(pack)
            if pack.get("sip") in client_dict.keys() and pack.get("tproto") == 1: #icmp
                client_dict[pack.get("sip")].append(pack)

        #print("client_dict", ["%s:%s"%(el,str(len(client_dict[el]))) for el in client_dict])
        #check if anyone is ping storming
        for client_ip in client_dict:
            #print("len cl packs", len(client_dict[client_ip]))
            #print(client_dict[client_ip][0], self.__convert_date_to_unix(client_dict[client_ip][0].get("arrive_time")))
            #print(client_dict[client_ip][1], self.__convert_date_to_unix(client_dict[client_ip][1].get("arrive_time")))
            client_dict[client_ip].sort(key=lambda pack: self.__convert_date_to_unix(pack.get("arrive_time")))
            packages = client_dict[client_ip]
            #print(len(packages))
            is_icmp_storm = self.__check_for_single_ping_storm(packages)
            if is_icmp_storm:
                result = {
                    "name": ICMPStormDetector.NAME,
                    "ip":client_ip,
                    "mac": self.__get_mac_by_ip(client_ip),
                    "type": type_for_guilty
                }
                #print("hope",client_ip)
                analysis_results.append(result)
        return analysis_results


    def execute(self,option, opt_str, value, parser):
        results_list = []
        output_file = value
        #print(output_file)
        if self.__can_operate:
            results = self.__analyse_input()
            results_list += results
            #print("results_list",results_list)
            self.__save_results_file(output_file, results_list)
        else:
            err = {"results": [{"error":self.__errors}]}
            with open(output_file,"w") as file:
                json.dump(err, file)

if __name__ == "__main__":
    usage = "usage: %prog [options] arg1"
    isd = ICMPStormDetector()
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("--name", callback = ICMPStormDetector.show_name,
                      action="callback", default=True,
                      help="prints the name of the heuristic")
    parser.add_option("--type", callback=ICMPStormDetector.show_type,
                      action="callback", dest="verbose", default=True,
                      help="prints the type of the heuristic")
    parser.add_option("--requirements", callback=ICMPStormDetector.show_requirements,
                      action="callback", dest="verbose", default=True,
                      help="prints the requirements of the heuristic")
    parser.add_option("--load", callback=isd.load,
                      type="string", dest="input_file",
                      action="callback",
                      help="loads the input file to the heuristic")
    parser.add_option("--execute", callback=isd.execute,
                      type="string", dest="output_file",
                      action="callback",
                      help="executes the heuristic and returns the results in the output_file")
    (options, args) = parser.parse_args()
