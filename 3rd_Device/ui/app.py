from flask import Flask, render_template, request
from flask_cors import CORS
from orm.infectivity_type import InfectivityTypes
from orm.client import Client
from orm.package import Package
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from logger.logger import Logger
from domain.test import Test as TestDOM
from orm.test_status import TestStatus

from datetime import datetime,timedelta
import random
import secrets
import html

app = Flask(__name__, static_url_path='/static', static_folder='static')
CORS(app)


def convert_test_status_to_string(test_status):
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

def generate_list_of_packs(nr_packs = 500):
    max_ip = 4
    t_proto_ls = [1, 6, 17, 2]
    chances_t_proto = [30, 30, 30, 10]
    input_time = datetime.utcnow()
    max_len_payload = 50
    resp = []
    start_date = datetime(2023, 6, 25, 23, 32, 55)
    end_date = datetime(2023, 6, 25, 23, 33, 55)
    time_diff = end_date - start_date
    for i in range(nr_packs):
        sip = ""
        dip = ""
        while sip == dip:
            sip = "192.168.1." + str(random.randint(1, max_ip))
            dip = "192.168.1." + str(random.randint(1, max_ip))
        sport = random.randint(1, 65535)
        dport = random.randint(1, 65535)
        t_proto = random.choices(t_proto_ls, chances_t_proto)[0]
        len_payload = random.randint(1, max_len_payload)
        payload = bytearray(secrets.token_bytes(len_payload))
        random_seconds = random.randint(0, int(time_diff.total_seconds()))
        random_datetime = start_date + timedelta(seconds=random_seconds)
        pack = Package(PackageID=i, SourceIP=sip, SourcePort=sport, DestinationIP=dip, DestinationPort=dport,
                       NetworkProtocol=4, TransportProtocol=t_proto, ApplicationProtocol=0,
                       ArriveTime=random_datetime, PayloadSize=len_payload, Payload=payload)
        resp.append(pack)
    return resp

def org_packs_by_t_proto(list_packages):
    t_proto_list = {}
    for pack in list_packages:
        if str(pack.TransportProtocol) not in t_proto_list.keys():
            t_proto_list[str(pack.TransportProtocol)] = 0
        t_proto_list[str(pack.TransportProtocol)] += 1
    return t_proto_list

def get_data_from_main_server(request):
    comm = ITC("127.0.0.1", 5004, Logger())
    comm.connect()
    comm.send_request(request)
    resp = comm.read_response()
    comm.close_connection()
    return resp

def send_data_to_main_server(request):
    comm = ITC("127.0.0.1", 5004, Logger())
    comm.connect()
    comm.send_request(request)
    comm.close_connection()

@app.route('/')
def index():
    #url_for('static', filename='css/main-page.css')
    return render_template('index.html')

@app.route('/home')
def home():
    type = request.args.get('type', None)
    state = request.args.get('state', None)
    print(type)
    if int(type) == 0:
        pack = InfectivityRequest(InfectivityRequestType.GET_CONNECTED_CLIENTS, [])
        resp = get_data_from_main_server(pack)
        lista_cl = resp.payload
        #print(lista_cl)

        pack = InfectivityRequest(InfectivityRequestType.GET_ALL_RUNNING_TESTS, [])
        resp = get_data_from_main_server(pack)
        lista_test = resp.payload
        #print(lista_test)

        pack = InfectivityRequest(InfectivityRequestType.GET_NETWORK_CONFIGURATION, [])
        resp = get_data_from_main_server(pack)
        configs = resp.payload[0]
        #print(configs)
        # lista_cl = [ Client(ClientID=1, CurrentIP= "192.168.1.4", MAC="00:11:22:33:44:55", IsConnected = 1, Score=20, IsTesting=0, InfectivityType=InfectivityTypes.DEFAULT.value),
        #           Client(ClientID=2, CurrentIP="192.168.1.6", MAC="34:45:78:ed:aa:bf", IsConnected=1, Score=200, IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_MAJOR.value),
        #           Client(ClientID=3, CurrentIP="192.168.1.7", MAC="aa:bb:cc:dd:ee:ff", IsConnected=1, Score=100,
        #                  IsTesting=1, InfectivityType=InfectivityTypes.INFECTED_MINOR.value),
        #           Client(ClientID=4, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=1, Score=0,
        #                  IsTesting=0, InfectivityType=InfectivityTypes.UNINFECTED.value),
        #              Client(ClientID=5, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=1, Score=0,
        #                     IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_SEVER.value)
        #           ]
        # lista_test = [TestDOM("192.168.1.4","2023-06-25 23:32:54",TestStatus.RUNNING,"",1),
        #               TestDOM("192.168.1.6", "2023-06-25 23:28:30", TestStatus.RUNNING, "", 2),
        #               TestDOM("192.168.1.7", "2023-06-25 21:45:12", TestStatus.RUNNING, "", 3)
        #          ]
        return render_template('home.html', lista_clients = lista_cl, lista_tests = lista_test, automatic= configs.get("automatic"), lockdown= configs.get("lockdown"))
    elif int(type) == 1:
        auto = state
        pack = InfectivityRequest(InfectivityRequestType.AUTOMATIC_SETTINGS, [True if int(auto) == 1 else False])
        #print(pack)
        send_data_to_main_server(pack)
        return {}
    elif int(type) == 2:
        lock = state
        pack = InfectivityRequest(InfectivityRequestType.LOCKDOWN_SETTINGS, [True if int(lock) == 1 else False])
        #print(pack)
        send_data_to_main_server(pack)
        return {}

@app.route('/history')
def history():
    # lista_cl = [ Client(ClientID=1, CurrentIP= "192.168.1.4", MAC="00:11:22:33:44:55", IsConnected = 1, Score=20, IsTesting=0, InfectivityType=InfectivityTypes.DEFAULT.value),
    #           Client(ClientID=2, CurrentIP="192.168.1.6", MAC="34:45:78:ed:aa:bf", IsConnected=1, Score=200, IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_MAJOR.value),
    #           Client(ClientID=3, CurrentIP="192.168.1.7", MAC="aa:bb:cc:dd:ee:ff", IsConnected=1, Score=100,
    #                  IsTesting=1, InfectivityType=InfectivityTypes.INFECTED_MINOR.value),
    #           Client(ClientID=4, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=1, Score=0,
    #                  IsTesting=0, InfectivityType=InfectivityTypes.UNINFECTED.value),
    #              Client(ClientID=5, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=1, Score=0,
    #                     IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_SEVER.value)
    #           ]
    # lista_test = [TestDOM("192.168.1.4","2023-06-25 23:32:54",TestStatus.RUNNING,"",1),
    #               TestDOM("192.168.1.6", "2023-06-25 23:28:30", TestStatus.RUNNING, "", 2),
    #               TestDOM("192.168.1.7", "2023-06-25 21:45:12", TestStatus.RUNNING, "", 3)
    #          ]
    #url_for('static', filename='css/main-page.css')
    return render_template('history.html')


@app.route('/heuristics')
def heuristic():
    pack = InfectivityRequest(InfectivityRequestType.GET_ALL_HEURISTICS, [])
    resp = get_data_from_main_server(pack)
    lista_heur = resp.payload
    #url_for('static', filename='css/main-page.css')
    list_hey_types = []
    list_hey_types.append({"name":"ceva","id_number":1, "description":"panama1"})
    list_hey_types.append({"name": "ceva2", "id_number": 2 ,"description":"panama2"})
    list_req_types=[]
    list_req_types.append({"name":"ceva","id_number":1, "description":"panama1", "code_example":{"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2}, "ratio": [2, 14213]}})
    list_req_types.append({"name": "ceva2", "id_number": 2, "description": "panama2", "code_example": {"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2},"ratio": [2, 14213]}})
    return render_template('heuristics.html', list_heur_types = list_hey_types, list_req_types = list_req_types, lista_heur = lista_heur)

@app.route('/package_clients')
def packages_clients():
    ip = request.args.get('ip', None)
    mac = request.args.get('mac', None)
    nr = int(request.args.get('nr', None))
    type = request.args.get('type', None)
    if int(type) == 0:
        return render_template('packages_client.html', ip=ip, mac = mac)
    list_packages_to_return = []
    list_packages = generate_list_of_packs(100)
    if int(type) == 1:
        #t_proto_list = org_packs_by_t_proto(list_packages)
        pack = InfectivityRequest(InfectivityRequestType.GET_PACKAGES_PROTO_STATS, [ip,nr])
        resp = get_data_from_main_server(pack)
        t_proto_list = resp.payload
        for el in t_proto_list:
            data_proto = {
                "t_proto":int(el),
                "nr_packs":t_proto_list[el]
            }
            list_packages_to_return.append(data_proto)
    if int(type) == 2:
        pack = InfectivityRequest(InfectivityRequestType.GET_LAST_NR_PACKAGES, [ip, nr])
        resp = get_data_from_main_server(pack)
        list_packages = resp.payload
        # list_packages.reverse()
        # list_packages=list_packages[0:nr]
        # list_packages.reverse()
        for el in list_packages:
            data_pack = {
                "id":str(el.PackageID),
                "details":f"[%s] | %s | %s:%s -> %s:%s | NP:%s, TP:%s, AP:%s | SIZE = %s" % \
                          (str(el.PackageID),el.ArriveTime,el.SourceIP,str(el.SourcePort),el.DestinationIP,str(el.DestinationPort), \
                           str(el.NetworkProtocol),str(el.TransportProtocol),str(el.ApplicationProtocol),str(el.PayloadSize)),
            }
            list_packages_to_return.append(data_pack)
    return list_packages_to_return


@app.route('/packages')
def packages():
    id = request.args.get('id', None)
    max_len_payload = 1000
    len_payload = random.randint(1, max_len_payload)
    payload = bytearray(secrets.token_bytes(len_payload))
    str_data = "".join([chr(bit) if 33 < bit < 126 else "." for bit in payload])
    str_data = str_data.replace("\\","\\\\")
    data_payload={
        "payload_hexa": "".join([str(hex(bit)).upper().replace("0X","") + " " for bit in payload]),

        "payload_str": html.escape(str_data)
    }
    return data_payload

@app.route('/tests')
def tests():
    # list_tests=[(TestDOM("192.168.1.1", "2023-06-25 23:32:54", TestStatus.FINISHED, "2023-06-25 23:32:56", 1),{"win.trojan.rat":2}),
    #             (TestDOM("192.168.1.1", "2023-06-25 23:28:30", TestStatus.FINISHED, "2023-06-25 23:32:40", 2),{"win.trojan.rat":2, "win.trojan.julius":1}),
    #             (TestDOM("192.168.1.1", "2023-06-25 21:45:12", TestStatus.RUNNING, "", 3),{})
    #               ]
    # test = {}
    id = int(request.args.get('id', None))
    print(id)
    pack = InfectivityRequest(InfectivityRequestType.GET_TEST, [id])
    resp = get_data_from_main_server(pack)

    test_db = resp.payload[0]
    malware = resp.payload[1]
    print(resp, test_db.id, test_db.time_started)
    # for el in list_tests:
    #     if el[0].id == int(id):
    test = {
        "id":test_db.id,
        "ip":test_db.client_ip,
        "time_start":str(test_db.time_started),
        "time_finished":str(test_db.time_finished),
        "status":convert_test_status_to_string(test_db.status.value).upper(),
        "results":malware
    }
    return test


    #url_for('static', filename='css/main-page.css')
    # list_hey_types = []
    # list_hey_types.append({"name":"ceva","id_number":1, "description":"panama1"})
    # list_hey_types.append({"name": "ceva2", "id_number": 2 ,"description":"panama2"})
    # list_req_types=[]
    # list_req_types.append({"name":"ceva","id_number":1, "description":"panama1", "code_example":{"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2}, "ratio": [2, 14213]}})
    # list_req_types.append({"name": "ceva2", "id_number": 2, "description": "panama2", "code_example": {"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2},"ratio": [2, 14213]}})


@app.route('/test_clients')
def test_clients():
    # lista_test = [TestDOM("192.168.1.1", "2023-06-25 23:32:54", TestStatus.FINISHED, "2023-06-25 23:32:56", 1),
    #               TestDOM("192.168.1.1", "2023-06-25 23:28:30", TestStatus.FINISHED, "2023-06-25 23:32:40", 2),
    #               TestDOM("192.168.1.1", "2023-06-25 21:45:12", TestStatus.RUNNING, "", 3)
    #               ]
    ip = request.args.get('ip', None)
    mac = request.args.get('mac', None)
    type = request.args.get('type', None)
    if int(type) == 0:
        pack = InfectivityRequest(InfectivityRequestType.GET_TESTS_FOR_CLIENT, [ip,mac])
        resp = get_data_from_main_server(pack)
        lista_test = resp.payload
        return render_template('tests_client.html', lista_tests = lista_test, ip=ip, mac=mac)
    lista_to_send = []
    if int(type) == 1:
        # lista = [{
        #         "nr_tests": 1,
        #         "day_of_test": "2023-06-25"
        #     },
        #     {
        #         "nr_tests": 2,
        #         "day_of_test": "2023-06-26"
        #     },
        #     {
        #         "nr_tests": 1,
        #         "day_of_test": "2023-06-27"
        #     }
        # ]
        pack = InfectivityRequest(InfectivityRequestType.GET_TESTS_FOR_CLIENT_PER_DAY, [ip, mac])
        resp = get_data_from_main_server(pack)
        lista = resp.payload
        for el in lista:
            plat_to_send = {
                "nr_tests": lista[el],
                "day_of_test": el
            }
            lista_to_send.append(plat_to_send)
    if int(type) == 2:
        # lista = [{
        #         "nr_malwares": 5,
        #         "day_of_test": "2023-06-25"
        #     },
        #     {
        #         "nr_malwares": 10,
        #         "day_of_test": "2023-06-26"
        #     },
        #     {
        #         "nr_malwares": 0,
        #         "day_of_test": "2023-06-27"
        #     }
        # ]
        pack = InfectivityRequest(InfectivityRequestType.GET_MALWARE_FOR_CLIENT_PER_DAY, [ip, mac])
        resp = get_data_from_main_server(pack)
        lista = resp.payload
        for el in lista:
            plat_to_send = {
                "nr_malwares": lista[el],
                "day_of_test": el
            }
            lista_to_send.append(plat_to_send)
    return lista_to_send
    #url_for('static', filename='css/main-page.css')
    # list_hey_types = []
    # list_hey_types.append({"name":"ceva","id_number":1, "description":"panama1"})
    # list_hey_types.append({"name": "ceva2", "id_number": 2 ,"description":"panama2"})
    # list_req_types=[]
    # list_req_types.append({"name":"ceva","id_number":1, "description":"panama1", "code_example":{"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2}, "ratio": [2, 14213]}})
    # list_req_types.append({"name": "ceva2", "id_number": 2, "description": "panama2", "code_example": {"malware": {"Win.Trojan.EmbeddedDotNetBinary-9940868-0": 2},"ratio": [2, 14213]}})


@app.route('/clients', methods=['GET', 'POST'])
def clients():
    # lista = [ Client(ClientID=1, CurrentIP= "192.168.1.4", MAC="00:11:22:33:44:55", IsConnected = 1, Score=20, IsTesting=0, InfectivityType=InfectivityTypes.DEFAULT.value),
    #           Client(ClientID=2, CurrentIP="192.168.1.6", MAC="34:45:78:ed:aa:bf", IsConnected=1, Score=200, IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_MAJOR.value),
    #           Client(ClientID=3, CurrentIP="192.168.1.7", MAC="aa:bb:cc:dd:ee:ff", IsConnected=1, Score=100,
    #                  IsTesting=1, InfectivityType=InfectivityTypes.INFECTED_MINOR.value),
    #           Client(ClientID=4, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=0, Score=0,
    #                  IsTesting=0, InfectivityType=InfectivityTypes.UNINFECTED.value)
    #           ]
    type = request.args.get('type', None)
    print(request.args)
    if request.method == "GET":
        if int(type) == 0:
            pack = InfectivityRequest(InfectivityRequestType.GET_ALL_CLIENTS, [])
            resp = get_data_from_main_server(pack)
            lista = resp.payload
            return render_template("clients.html", lista = lista)
        if int(type) == 1:
            ip = request.args.get('ip', None)
            pack = InfectivityRequest(InfectivityRequestType.REACH_CLIENT, [ip])
            resp = get_data_from_main_server(pack)
            is_reachable = resp.payload[0]
            return {"reach": is_reachable}
        if int(type) == 2:
            ip = request.args.get('ip', None)
            pack = InfectivityRequest(InfectivityRequestType.SCAN_CLIENT, [ip])
            resp = get_data_from_main_server(pack)
            is_scan = resp.payload[0]
            return {"scan": is_scan}
    elif request.method == "POST":
        print(request.get_json())
        if int(type) == 3:
            ip = request.get_json().get('ip', None)
            mac = request.get_json().get('mac', None)
            score = request.get_json().get('score', None)
            infec_type = request.get_json().get('infec_type', None)
            print([ip,mac,score,infec_type])
            pack = InfectivityRequest(InfectivityRequestType.TRANSFER_CLIENT_EXTENDED, [ip,mac,score,infec_type])
            resp = get_data_from_main_server(pack)
            is_transfered = resp.payload[0]
            return {"transfered":is_transfered}
    return {}

@app.route('/malware/<int:type>')
def malware(type:int):
    if int(type) == 0:
        return render_template('malware.html')
    else:
        lista = []
        comm = ITC("127.0.0.1", 5004, Logger())
        comm.connect()
        if int(type) == 1:
            pack = InfectivityRequest(InfectivityRequestType.GET_PLATFORMS,[])
            comm.send_request(pack)
            resp = comm.read_response()
            if resp is not None:
                for plat in resp.payload:
                    plat_to_send ={
                        "id":plat.PlatformID,
                        "name": plat.Name,
                        "score":plat.Score
                    }
                    lista.append(plat_to_send)
        if int(type) == 2:
            pack = InfectivityRequest(InfectivityRequestType.GET_CATEGORIES,[])
            comm.send_request(pack)
            resp = comm.read_response()
            if resp is not None:
                for cat in resp.payload:
                    cat_to_send ={
                        "id":cat.CategoryID,
                        "name": cat.Name,
                        "score":cat.Score
                    }
                    lista.append(cat_to_send)
        if int(type) == 3 or int(type) == 4:
            pack = InfectivityRequest(InfectivityRequestType.GET_SAMPLE_STATS,[])
            comm.send_request(pack)
            resp = comm.read_response()
            resp_list = resp.payload[0] if int(type) == 3 else resp.payload[1]
            if resp is not None:
                for el in resp_list:
                    el_to_send ={
                        "name":el,
                        "value": resp_list.get(el)
                    }
                    lista.append(el_to_send)
        comm.close_connection()
        return lista
