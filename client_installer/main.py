# This is a sample Python script.

# Press ⌃R to execute it or replace it with your code.
# Press Double ⇧ to search everywhere for classes, files, tool windows, actions, and settings.
from network.server import *
import os
import sys
import netifaces
import ipaddress
from time import sleep
from signal import SIGKILL
from subprocess import run

port = 5004
pid_file = os.getcwd()+"\\"+".pid"

def check_for_subnet_belongs(ip, subnet):
    ip = ipaddress.ip_address(ip)
    network = ipaddress.ip_network(subnet)

    return ip in network

def get_device_ip(subnet):
    try:
        interfaces = netifaces.interfaces()
        for interface in interfaces:
            if_addresses = netifaces.ifaddresses(interface)
            if netifaces.AF_INET in if_addresses:
                ip_address = if_addresses[netifaces.AF_INET][0]['addr']
                if check_for_subnet_belongs(ip_address,subnet):
                    return ip_address
    except (KeyError, ValueError) as e:
        print(f"Error getting IP address: {e}")

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    #test_test_package()
    #test_scan_package()
    if len(sys.argv) >= 2:
        if sys.argv[1] == "-start":
            if len(sys.argv) >= 3:
                subnet = sys.argv[2]
                host = get_device_ip(subnet)
                logger = Logger()
                server = Server(host, port, logger)
                pid = os.fork()
                if pid > 0:
                    with open(pid_file,"w") as file:
                        file.write(str(pid)+"\n")
                    sleep(1)
                elif pid == 0:
                    os.setsid()
                    server.run_server()
                else:
                    print("Cannot start server")
            else:
                print("Missing subnet/netmask")
        elif sys.argv[1] == "-stop":
            pid = None
            with open(pid_file, "r") as file:
                pid = int(file.readline())
            if pid is not None:
                os.kill(pid, SIGKILL)
            else:
                print("Cannot stop process")
        else:
            print("Invalid command, try -start or -stop")

#
# # See PyCharm help at https://www.jetbrains.com/help/pycharm/
# import multiprocessing
# import time
# import signal
# import os
#
# def my_function():
#     while True:
#         print("Daemon process running")
#         time.sleep(1)
#
# def sigterm_handler(signum, frame):
#     print("Received SIGTERM signal. Exiting daemon process.")
#     exit(0)
#
# if __name__ == '__main__':
#     daemon_process = multiprocessing.Process(target=my_function)
#     daemon_process.daemon = True
#
#     daemon_process.start()
#     print(daemon_process.pid)
#     print(daemon_process.name)
#     print(multiprocessing.current_process().pid)
#
#     print("Main process started")
#
#     # Register SIGTERM handler
#     signal.signal(signal.SIGTERM, sigterm_handler)
#
#     while True:
#         try:
#             time.sleep(1)
#         except KeyboardInterrupt:
#             print("Received KeyboardInterrupt. Exiting main process.")
#             break
#
#     print("Main process exited")
