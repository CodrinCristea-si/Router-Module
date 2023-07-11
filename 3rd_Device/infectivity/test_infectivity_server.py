import pickle
import threading
from multiprocessing import Process
import socket
from time import sleep

from infectivity.infectivity_server import InfectivityServer
from packages.infectivity_request import InfectivityRequest,InfectivityRequestType
from logger.logger import Logger

HOST = "127.0.0.1"
PORT = 5000
log = Logger()

def create_test(ip,mac):
    for i in range(5):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #log.info("created")
        s.connect((HOST, PORT))
        #log.info("connected")
        pack = InfectivityRequest(InfectivityRequestType.ADD_CLIENT, [ip, mac])
        #log.info("pack")
        pack_by = pickle.dumps(pack)
        #log.info("dump")
        s.sendall(pack_by)
        #log.info("sent")
        s.close()
        #log.info("close")

def start_server(serv):
    serv.run_server()

if __name__ == "__main__":


    serv = InfectivityServer(HOST,PORT,log)

    ip = "192.168.0.1"
    mac = "aa:bb:cc:dd:ee:ff"
    p2 = Process(target=create_test, args=(ip, mac))
    #p1= Process(target=start_server, args=(serv,))
    p1 = threading.Thread(target=start_server, args=(serv,))
    p1.start()
    p2.start()
    sleep(1)
    for i in range(5):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        pack = InfectivityRequest(InfectivityRequestType.REMOVE_CLIENT, [ip, mac])
        pack_by = pickle.dumps(pack)
        s.sendall(pack_by)
        s.close()
    p2.join()
    p1.join()