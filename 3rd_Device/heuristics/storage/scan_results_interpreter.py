
"""
{type:PackageType.RESULTS, payload:{'malware': {'Win.Trojan.EmbeddedDotNetBinary-9940868-0': 2}, 'ratio': [2, 14213]}}
"""

import sys
import optparse
import json
import os
from datetime import datetime
import calendar


class ScanResultsInterpreter:
    NAME = "Interpret Client Scan Results"
    TYPE = 1
    REQUIREMENTS = [4, 6, 7, 8]
    TMP_FILE = "tmp_Interpret_Client_Scan_Results_"

    def __init__(self):
        self.__client_results = []
        self.__requirements = {}
        self.__errors = []
        self.__can_operate = False


    @staticmethod
    def show_name(option, opt_str, value, parser):
        print(ScanResultsInterpreter.NAME)

    @staticmethod
    def show_type(option, opt_str, value, parser):
        print(ScanResultsInterpreter.TYPE)

    @staticmethod
    def show_requirements(option, opt_str, value, parser):
        print("".join([str(el)+" " for el in ScanResultsInterpreter.REQUIREMENTS]))

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
        for req in ScanResultsInterpreter.REQUIREMENTS:
            is_req_satisfied = False
            for req_input in self.__requirements.get("response"):
                if req_input.get("type") is not None and req_input.get("type") == req and len(req_input.get("input")) >= 0:
                    is_req_satisfied = True
            if not is_req_satisfied:
                self.__errors.append("Requirement %d not satisfied!" % (req))
        if len(self.__errors) != 0:
            return False
        return True

    def __generate_file(self,basic_title:str):
        dt = datetime.now()
        utc_time = calendar.timegm(dt.utctimetuple())
        return str(basic_title) + str(utc_time)

    def __save_load_file(self, data):
        os.remove("%s\\%s*"%(os.getcwd(),ScanResultsInterpreter.TMP_FILE))
        tmp_file = os.getcwd() + "\\" + self.__generate_file(ScanResultsInterpreter.TMP_FILE)
        os.system(f"type nul> {tmp_file}")
        with open(tmp_file, "w") as file:
            json.dump(data, file)

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


    def __parse_malware_name(self,mal_name):
        data = mal_name.split("-")[0]  # skip ip and revision code
        # print(data)
        details = data.split(".")
        platform, category, name = ["","",""]
        if details[0] == "BC":
            details = details[1:]
        if len(details) > 3:
            platform, category, name, _ = details
        elif len(details) == 3:
            platform, category, name = details

        # if platform is not None and samples.get(platform) is None:
        #     samples[platform] = {}
        # if category is not None and samples[platform].get(category) is None:
        #     samples[platform][category] = set()
        if name is not None:
            if not name.isnumeric():
                details = name.split("_")
                if len(details) == 1:
                    name = name
                    #samples[platform][category].add(name)
                else:
                    acro, type = ["", ""]
                    if len(details) == 2:
                        acro, type = name.split("_")
                    elif len(details) == 3:
                        acro, type, _ = name.split("_")
                    elif len(details) == 4:
                        acro, type, _, _ = name.split("_")
                    if acro.isnumeric():
                        name = "~"
                        # samples[platform][category].add("~")
                    elif type != "ID":
                        name = acro + "_" + type
                        # samples[platform][category].add(acro + "_" + type)
                    else:
                        name = acro
                        # samples[platform][category].add(acro)
            else:
                name = "~"
                # samples[platform][category].add("~")
        return platform, category, name

    def __get_results_from_requirements(self):
        for req in self.__requirements.get("response"):
            print(req.get("type"))
            if req.get("type") == 4:
                return req.get("input")

    def __get_bonus(self, ratios):
        each_infected = 20
        ration_infected_per_total = 1000
        infected_files = ratios[0]
        total_files = ratios[1]
        bonus_score = infected_files *  each_infected // total_files //ration_infected_per_total
        return bonus_score

    def __get_list_according_to_req(self,req_type):
        list_to_return = []
        for req_input in self.__requirements.get("response"):
            if req_input.get("type") is not None and req_input.get("type") == req_type:
                list_to_return = req_input.get("input")
        return list_to_return

    def __get_malware_platform(self,name):
        plat = None
        list_platforms = self.__get_list_according_to_req(6)
        for el in list_platforms:
            if el.get("name") == name:
                plat = el
                break
        return plat

    def __get_malware_category(self,name):
        cat = None
        list_categories = self.__get_list_according_to_req(7)
        for el in list_categories:
            if el.get("name") == name:
                cat = el
                break
        return cat

    def __get_malware_sample(self,platform, category, name):
        plat = self.__get_malware_platform(platform)
        cat = self.__get_malware_category(category)
        samp = None
        if cat is not None and plat is not None:
            list_samples = self.__get_list_according_to_req(8)
            for el in list_samples:
                if el.get("name") == name and el.get("id_category") == cat.get("id") and el.get("id_platform") == plat.get("id"):
                    samp = el
                    break
        return samp

    def __analyse_results(self):
        score = 0
        details =  self.__get_results_from_requirements()[0]
        for mal in details.get("results").get("malware"):
            platform, category, name = self.__parse_malware_name(mal)
            print(platform, category, name)
            number_of_kind = details.get("results").get("malware").get(mal)
            sample = self.__get_malware_sample(platform, category, name)
            print(sample)
            score = 0
            if sample is None: # add the sample if it does not exist
                plat = self.__get_malware_platform(platform)
                cat = self.__get_malware_category(category)
                if plat is not None and cat is not None:
                    score += (plat.get("score") + cat.get("score")) * number_of_kind
                else:
                    score += 200
            else:
                score += sample.get("score") * number_of_kind
            score += self.__get_bonus(details.get("results").get("ratio"))
            print(score)
        analysis_results ={
            "name": ScanResultsInterpreter.NAME,
            "ip":details.get("ip"),
            "mac": details.get("mac"),
            "score": score
        }
        return analysis_results

    def __save_results_file(self,output_file, results_list):
        results_final = {
            "results": results_list
        }
        with open(output_file,"w") as file:
            json.dump(results_final, file)

    def execute(self,option, opt_str, value, parser):
        results_list = []
        output_file = value
        print(output_file)
        if self.__can_operate:
            results = self.__analyse_results()
            results_list.append(results)
            self.__save_results_file(output_file, results_list)
        else:
            err = {"results": [{"error":self.__errors}]}
            with open(output_file,"w") as file:
                json.dump(err, file)

if __name__ == "__main__":
    usage = "usage: %prog [options] arg1"
    sri = ScanResultsInterpreter()
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("--name", callback = ScanResultsInterpreter.show_name,
                      action="callback", default=True,
                      help="prints the name of the heuristic")
    parser.add_option("--type", callback=ScanResultsInterpreter.show_type,
                      action="callback", dest="verbose", default=True,
                      help="prints the type of the heuristic")
    parser.add_option("--requirements", callback=ScanResultsInterpreter.show_requirements,
                      action="callback", dest="verbose", default=True,
                      help="prints the requirements of the heuristic")
    parser.add_option("--load", callback=sri.load,
                      type="string", dest="input_file",
                      action="callback",
                      help="loads the input file to the heuristic")
    parser.add_option("--execute", callback=sri.execute,
                      type="string", dest="output_file",
                      action="callback",
                      help="executes the heuristic and returns the results in the output_file")
    (options, args) = parser.parse_args()

    # parser.add_option("-q", "--quiet",
    #                   action="store_false", dest="verbose",
    #                   help="be vewwy quiet (I'm hunting wabbits)")
    # parser.add_option("-f", "--filename",
    #                   metavar="FILE", help="write output to FILE")
    # parser.add_option("-m", "--mode",
    #                   default="intermediate",
    #                   help="interaction mode: novice, intermediate, "
    #                        "or expert [default: %default]")