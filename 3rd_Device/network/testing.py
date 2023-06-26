import os
import signal
import subprocess
from signal import Signals
import sys
import threading
from time import sleep
from concurrent.futures import ThreadPoolExecutor

from network.server import Server
from network.client import Client
from logger.logger import Logger
from threading import Thread
import multiprocessing

host = "localhost"
port = 1234


def test_test_package():
    logger = Logger()
    server = Server(host, port,logger)
    client = Client(host, port, logger)

    pid = os.fork()
    if pid > 0:
        sleep(1)
        client.connect()
        client.send_test_package()
        os.kill(pid,signal.SIGKILL)
        # cmd = ("kill -9 `lsof -t -i:%s`" % (str(port))).split(" ")
        # subprocess.Popen(cmd)
        sleep(1)
    else:
        server.run_server()


def test_scan_package():
    logger = Logger()
    server = Server(host, port, logger)
    client = Client(host, port, logger)

    pid = os.fork()
    if pid > 0:
        sleep(1)
        client.connect()
        client.send_scan_package()
        os.kill(pid, signal.SIGKILL)
    else:
        server.run_server()



