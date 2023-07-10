

import sys
import optparse
import json
import os
from datetime import datetime
import calendar


class UnknownBehaviourInterpreter:
    NAME = "Unknown Behaviour Detection Heuristic"
    TYPE = 3
    REQUIREMENTS = [1, 3, 5, 9]

    __GENERAL_CASE = {
            "heur_potential_catcher":None,
            "target_client": None,
            "is_client_being_tested": None,
            "test_for_client": None,
            "test_has_results":None,
            "event_repeats":None,
            "the_same_client":None,
            "is_list_arguments_completed": False,
            "is_arguments_completed":False,
            "is_case_analysed": None,
            "is_same_client_targeted_by_analyse":None,
            "is_case_completed": False
        }

    def __init__(self):
        self.__client_results = []
        self.__requirements = {}
        self.__errors = []
        self.__can_operate = False


    @staticmethod
    def show_name(option, opt_str, value, parser):
        print(UnknownBehaviourInterpreter.NAME)

    @staticmethod
    def show_type(option, opt_str, value, parser):
        print(UnknownBehaviourInterpreter.TYPE)

    @staticmethod
    def show_requirements(option, opt_str, value, parser):
        print("".join([str(el)+" " for el in UnknownBehaviourInterpreter.REQUIREMENTS]))

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
        for req in UnknownBehaviourInterpreter.REQUIREMENTS:
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

    def __convert_date_to_unix(self, date_str):
        input_date_time_str = date_str
        input_date_time = datetime.strptime(input_date_time_str, '%Y-%m-%d %H:%M:%S')
        unix_time = input_date_time.timestamp()
        return unix_time

    def __save_results_file(self,output_file, results_list):
        results_final = {
            "results": results_list
        }
        with open(output_file,"w") as file:
            json.dump(results_final, file)


    def __get_element_by_id(self, elem_id, list_elem, field):
        elem = None
        for el in list_elem:
            if el.get(field) == elem_id:
                elem = el
                break
        return elem

    def __get_current_heuristic_from_heur_list(self):
        list_heur = self.__get_list_according_to_req(9)
        heur = None
        for el in list_heur:
            if el.get("name") == UnknownBehaviourInterpreter.NAME:
                heur = el
        return heur

    def __update_case(self,current_heur,potential_danger, hist, lists_to_search):
        list_heur, list_clients, list_tests = lists_to_search[0], lists_to_search[1], lists_to_search[2]
        # search for a potential package heuristic that found an anomaly
        if potential_danger.get("heur_potential_catcher") is None and hist.get("type") == "HEURISTIC_HAS_RESULTS":
            heur_id = hist.get("target_id")
            heur = self.__get_element_by_id(heur_id, list_heur, "id")
            if heur.get("type") == 2:  # only packages type heuristics
                potential_danger["heur_potential_catcher"] = heur
        # search for the client that produces that anomaly
        if potential_danger.get("heur_potential_catcher") is not None and potential_danger.get(
                "target_client") is None and hist.get("type") == "HEURISTIC_PROBLEM_FOUND_CLIENT":
            client_id = hist.get("target_id")
            potential_danger["target_client"] = self.__get_element_by_id(client_id, list_clients, "id")
        # search for the test for that client
        if potential_danger.get("target_client") is not None and potential_danger.get(
                "is_client_being_tested") is None and hist.get("type") == "CLIENT_SCAN_BEGIN":
            client_id = hist.get("target_id")
            if client_id == potential_danger.get("target_client").get("id"):
                potential_danger["is_client_being_tested"] = True

        if potential_danger.get("is_client_being_tested") is not None and potential_danger.get(
                "is_client_being_tested") and potential_danger.get("test_for_client") is None and hist.get(
                "type") == "TEST_ADDED":
            test_id = hist.get("target_id")
            potential_danger["test_for_client"] = self.__get_element_by_id(test_id, list_tests, "id")
            # check if the scan is complete
            if potential_danger["test_for_client"] is not None and potential_danger["test_for_client"].get(
                    "status") == "finished":
                # check if the test has result
                if len(potential_danger["test_for_client"].get("results")) == 0:
                    potential_danger["test_has_results"] = False
                else:
                    potential_danger["test_has_results"] = True

        # check if the same heuristic detect the behaviour after scanning ...
        if potential_danger.get("test_has_results") is not None and not potential_danger.get(
                "test_has_results") and potential_danger.get("event_repeats") is None and hist.get(
            "type") == "HEURISTIC_HAS_RESULTS":
            heur_id = hist.get("target_id")
            if heur_id == potential_danger["heur_potential_catcher"].get("id"):
                potential_danger["event_repeats"] = True
            else:
                potential_danger["event_repeats"] = False

        # ... for the same client
        if potential_danger.get("event_repeats") is not None and potential_danger.get(
                "event_repeats") and potential_danger.get("the_same_client") is None and hist.get(
            "type") == "HEURISTIC_PROBLEM_FOUND_CLIENT":
            client_id = hist.get("target_id")
            if client_id == potential_danger["target_client"].get("id"):
                potential_danger["the_same_client"] = True
            else:
                potential_danger["the_same_client"] = False

        list_options = ["heur_potential_catcher", "target_client", "is_client_being_tested", "test_for_client",
                        "test_has_results", "event_repeats", "the_same_client"]
        is_list_options_completed = True
        for el in list_options:
            if potential_danger.get(el) is None:
                is_list_options_completed = False
                break
        potential_danger["is_list_arguments_completed"] = is_list_options_completed
        if potential_danger.get("is_list_arguments_completed"):
            potential_danger["is_arguments_completed"] = True

        # check if this case has been analysed
        if potential_danger.get("is_arguments_completed") is not None and potential_danger.get(
                "is_arguments_completed") and potential_danger.get("is_case_analysed") is None and hist.get(
            "type") == "HEURISTIC_HAS_RESULTS":
            heur_id = hist.get("target_id")
            if heur_id == current_heur.get("id"):
                potential_danger["is_case_analysed"] = True
            else:
                potential_danger["is_case_analysed"] = False
                potential_danger["is_case_completed"] = True

        # check if there are measures taken
        if potential_danger.get("is_case_analysed") is not None and potential_danger.get(
                "is_case_analysed") and potential_danger.get("is_same_client_targeted_by_analyse") is None and hist.get(
            "type") == "HEURISTIC_PROBLEM_FOUND_CLIENT":
            client_id = hist.get("target_id")
            if client_id == potential_danger["target_client"].get("id"):
                potential_danger["is_same_client_targeted_by_analyse"] = True
            else:
                potential_danger["is_same_client_targeted_by_analyse"] = False
            potential_danger["is_case_completed"] = True

        return potential_danger
        # if potential_danger.get("is_same_client_targeted_by_analyse"):
        # if potential_danger.get("test_for_client") is not None and potential_danger.get("is_test_running") is None and hist.get("type") == "TEST_BEGUN":
        #     test_id = hist.get("target_id")
        #     if test_id == potential_danger.get("test_for_client").get("id"):
        #         potential_danger["is_test_running"] = True
        # if potential_danger.get("is_test_running") is not None and potential_danger.get("is_test_running") and potential_danger.get("is_test_finished") is None and hist.get("type") == "TEST_FINISHED":
        #     test_id = hist.get("target_id")
        #     if test_id == potential_danger.get("test_for_client").get("id"):
        #         potential_danger["is_test_finished"] = True

    def __is_hist_type_to_be_analysed(self,hist):
        return hist.get("type") in ["HEURISTIC_HAS_RESULTS", "HEURISTIC_PROBLEM_FOUND_CLIENT", "CLIENT_SCAN_BEGIN", "TEST_ADDED"]


    def __can_args_attr_be_filled(self,case, attr):
        # {
        #     "heur_potential_catcher": None,
        #     "target_client": None,
        #     "is_client_being_tested": None,
        #     "test_for_client": None,
        #     "test_has_results": None,
        #     "event_repeats": None,
        #     "the_same_client": None,
        #     "is_list_arguments_completed": False,
        #     "is_arguments_completed": False,
        #     "is_case_analysed": False,
        #     "is_same_client_targeted_by_analyse": False,
        #     "is_case_completed": False
        # }
        if attr == "heur_potential_catcher":
            if case.get("heur_potential_catcher") is None:
                return True
        if attr == "target_client":
            if case.get("heur_potential_catcher") is not None and case.get("target_client") is None:
                return True
        if attr == "is_client_being_tested":
            if case.get("target_client") is not None and case.get("is_client_being_tested") is None:
                return True
        if attr == "test_for_client":
            if case.get("is_client_being_tested") is not None and case.get("is_client_being_tested") and case.get("test_for_client") is None:
                return True
        if attr == "event_repeats":
            if case.get("test_has_results") is not None and not case.get("test_has_results") and case.get("event_repeats") is None:
                return True
        if attr == "the_same_client":
            if case.get("event_repeats") is not None and case.get("event_repeats") and case.get("the_same_client") is None:
                return True
        if attr == "is_case_analysed":
            if case.get("is_arguments_completed") is not None and case.get("is_arguments_completed") and case.get("is_case_analysed") is None:
                return True
        if attr == "is_same_client_targeted_by_analyse":
            if case.get("is_case_analysed") is not None and case.get("is_case_analysed") and case.get("is_same_client_targeted_by_analyse") is None:
                return True
        return False

    def __check_if_heuristic_is_of_type(self,list_heur, heur_id, type_heur):
        for heur in list_heur:
            if heur.get("id") == heur_id and heur.get("type")==type_heur:
                return True
        return False

    def __is_hist_useful_for_case(self,case,hist,lists_to_search):
        list_heur, list_clients, list_tests = lists_to_search[0], lists_to_search[1], lists_to_search[2]
        current_heur = self.__get_current_heuristic_from_heur_list()
        if current_heur is None:
            return False
        if hist.get("type") == "HEURISTIC_HAS_RESULTS":
            if self.__can_args_attr_be_filled(case, "is_case_analysed") and \
                    case.get("heur_potential_catcher").get("id") == current_heur.get("id"):
                return True

            elif self.__can_args_attr_be_filled(case,"event_repeats") and \
                    case.get("heur_potential_catcher").get("id") == hist.get("target_id"):
                return True
            elif self.__can_args_attr_be_filled(case,"heur_potential_catcher") and \
                    self.__check_if_heuristic_is_of_type(list_heur,hist.get("target_id"),2):
                return True
        elif hist.get("type") == "HEURISTIC_PROBLEM_FOUND_CLIENT":
            if self.__can_args_attr_be_filled(case, "is_same_client_targeted_by_analyse") and \
                    case.get("target_client").get("id") == hist.get("target_id"):
                return True

            elif self.__can_args_attr_be_filled(case,"the_same_client") and \
                    case.get("target_client").get("id") == hist.get("target_id"):
                return True
            elif self.__can_args_attr_be_filled(case,"target_client"):
                return True
        elif hist.get("type") == "CLIENT_SCAN_BEGIN":
            if self.__can_args_attr_be_filled(case,"is_client_being_tested") and \
                    case.get("target_client").get("id") == hist.get("target_id"):
                return True
        elif hist.get("type") == "TEST_ADDED":
            if self.__can_args_attr_be_filled(case,"test_for_client"):
                return True
        return False

    def __get_index_case_that_needs_hist(self,list_cases,hist, lists_to_search):
        index_case_needed = -1
        current_index = 0
        for case in list_cases:
            if self.__is_hist_useful_for_case(case,hist, lists_to_search):
                #case_needed = case
                index_case_needed = current_index
                break
            current_index += 1
        return index_case_needed

    def __get_complete_status(self,case):
        status = 0
        if case.get("heur_potential_catcher") is not None:
            status +=1
        if case.get("target_client") is not None:
            status +=1
        if case.get("is_client_being_tested") is not None:
            status +=1
        if case.get("test_for_client") is not None:
            status +=1
        if case.get("test_has_results") is not None:
            status +=1
        if case.get("event_repeats") is not None:
            status +=1
        if case.get("the_same_client") is not None:
            status +=1
        if case.get("is_list_arguments_completed"):
            status +=1
        if case.get("is_arguments_completed"):
            status +=1
        if case.get("is_case_analysed") is not None:
            status +=1
        if case.get("is_same_client_targeted_by_analyse") is not None:
            status +=1
        if case.get("is_case_completed"):
            status +=1
        return status

    def __analyse_input(self):
        # if unusual behaviour detected by heuristics then test
        # if test has no results then check if the same heuristic detected the same behaviour again
        # if the current heuristic did not detect this anomaly then is a case of INFECTIVITY_SEVER
        type_for_guilty = 5 # INFECTED SEVER
        current_heur = self.__get_current_heuristic_from_heur_list()
        if current_heur is None:
            return [{}]
        # check history list
        list_hist = self.__get_list_according_to_req(5)
        list_hist.sort(key=lambda hist: hist.get("id"))

        list_heur = self.__get_list_according_to_req(9)
        list_clients = self.__get_list_according_to_req(1)
        list_tests = self.__get_list_according_to_req(3)
        lists_to_search = [list_heur, list_clients, list_tests]
        list_cases = []
        list_results = []
        for hist in list_hist:
            if self.__is_hist_type_to_be_analysed(hist):
                #print(hist)
                index_case = self.__get_index_case_that_needs_hist(list_cases, hist, lists_to_search)
                if index_case == -1:
                    list_cases.append(UnknownBehaviourInterpreter.__GENERAL_CASE.copy())
                    index_case = len(list_cases) - 1
                case_to_update = list_cases[index_case]
                case_upgraded = self.__update_case(current_heur,case_to_update,hist,lists_to_search)
                list_cases[index_case] = case_upgraded
                completed_cases = [el for el in list_cases if el.get("is_case_completed")]
                for case in completed_cases:
                    if case.get("is_same_client_targeted_by_analyse"):
                        client = self.__get_element_by_id(case.get("target_client").get("id"),list_clients,"id")
                        result = {
                            "name": UnknownBehaviourInterpreter.NAME,
                            "ip": client.get("ip"),
                            "mac": client.get("mac"),
                            "type": type_for_guilty
                        }
                        list_results.append(result)
                    #print("completed", case)
                    list_cases.remove(case)
                #print([self.__get_complete_status(el)for el in list_cases])
        for case in list_cases:
            if case.get("is_arguments_completed") is not None and case.get("is_arguments_completed"):
                client = self.__get_element_by_id(case.get("target_client").get("id"), list_clients, "id")
                result = {
                    "name": UnknownBehaviourInterpreter.NAME,
                    "ip": client.get("ip"),
                    "mac": client.get("mac"),
                    "type": type_for_guilty
                }
                list_results.append(result)
        return list_results



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
    ub = UnknownBehaviourInterpreter()
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("--name", callback = UnknownBehaviourInterpreter.show_name,
                      action="callback", default=True,
                      help="prints the name of the heuristic")
    parser.add_option("--type", callback=UnknownBehaviourInterpreter.show_type,
                      action="callback", dest="verbose", default=True,
                      help="prints the type of the heuristic")
    parser.add_option("--requirements", callback=UnknownBehaviourInterpreter.show_requirements,
                      action="callback", dest="verbose", default=True,
                      help="prints the requirements of the heuristic")
    parser.add_option("--load", callback=ub.load,
                      type="string", dest="input_file",
                      action="callback",
                      help="loads the input file to the heuristic")
    parser.add_option("--execute", callback=ub.execute,
                      type="string", dest="output_file",
                      action="callback",
                      help="executes the heuristic and returns the results in the output_file")
    (options, args) = parser.parse_args()
