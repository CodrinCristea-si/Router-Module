from flask import Flask, render_template, request, url_for
from flask_cors import CORS
import orm
from infectivity.infectivity_type import InfectivityTypes
from orm.client import Client
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from packages.infectivity_response import *
from db import *
from logger.logger import Logger

app = Flask(__name__, static_url_path='/static', static_folder='static')
CORS(app)

@app.route('/')
def index():
    #url_for('static', filename='css/main-page.css')
    return render_template('index.html')

@app.route('/clients')
def clients():
    # lista = [ Client(ClientID=1, CurrentIP= "192.168.1.4", MAC="00:11:22:33:44:55", IsConnected = 1, Score=20, IsTesting=0, InfectivityType=InfectivityTypes.DEFAULT.value),
    #           Client(ClientID=2, CurrentIP="192.168.1.6", MAC="34:45:78:ed:aa:bf", IsConnected=1, Score=200, IsTesting=0, InfectivityType=InfectivityTypes.INFECTED_MAJOR.value),
    #           Client(ClientID=3, CurrentIP="192.168.1.7", MAC="aa:bb:cc:dd:ee:ff", IsConnected=1, Score=100,
    #                  IsTesting=1, InfectivityType=InfectivityTypes.INFECTED_MINOR.value),
    #           Client(ClientID=4, CurrentIP="192.168.1.10", MAC="11:ff:44:aa:dd:55", IsConnected=0, Score=0,
    #                  IsTesting=0, InfectivityType=InfectivityTypes.UNINFECTED.value)
    #           ]
    comm = ITC("127.0.0.1", 5004, Logger())
    comm.connect()
    pack = InfectivityRequest(InfectivityRequestType.GET_ALL_CLIENTS, [])
    comm.send_request(pack)
    resp = comm.read_response()
    lista = resp.payload
    return render_template("clients.html", lista = lista)

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
        return lista
