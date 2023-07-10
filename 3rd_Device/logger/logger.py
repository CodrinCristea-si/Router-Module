import logging
import logging.handlers
import os
import sys
import multiprocessing
from multiprocessing.queues import SimpleQueue
from queue import Queue


class Logger:

    def __init__(self, default_level:int = logging.INFO, log_file:str = "logs.txt"):
        #self.__logger = logging.getLogger()
        self.__logger = multiprocessing.get_logger()
        self.__logger.propagate=True
        self.__level = default_level
        self.__logger.setLevel(default_level)
        self.__file = log_file
        self.__formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(thread)d : %(name)s -- %(funcName)s : %(message)s')

        self.initialize()

    def get_log_file_path(self):
        return os.getcwd() + "\\" + self.__file

    def initialize(self) -> None:
        self.__queue = multiprocessing.Queue(-1)
        self.__handler = logging.handlers.QueueHandler(self.__queue)
        self.__logger.addHandler(self.__handler)

        # Create a QueueListener to forward log messages from all processes
        # to the same set of handlers
        self.__listener = logging.handlers.QueueListener(self.__queue, *logging.root.handlers)
        self.__listener.start()

        # Add console handler
        console_printer = logging.StreamHandler()
        console_printer.setLevel(self.__level)
        console_printer.setFormatter(self.__formatter)
        self.__logger.addHandler(console_printer)

        # Add file handler if necessary
        file_printer = logging.FileHandler(self.__file)
        file_printer.setLevel(self.__level)
        file_printer.setFormatter(self.__formatter)
        self.__logger.addHandler(file_printer)
        # print(self.__logger.handlers)
        # print(self.__logger.isEnabledFor(logging.INFO))
        # print(self.__logger.isEnabledFor(logging.WARNING))
        # self.__logger.info("Init")
        #self.__logger.log(logging.NOTSET,"=====================================")

    def log(self, level:int, message:str) -> None:
        self.__logger.log(level, message)

    def info(self, message:str) -> None:
        self.__logger.info(message)

    def warning(self, message:str) -> None:
        self.__logger.warning(message)

    def error(self, message:str) -> None:
        self.__logger.error(message)

    def debug(self, message:str) -> None:
        self.__logger.debug(message)


"""
def __init__(self, name):
        self.name = name
        self.logger = logging.getLogger(name)
        self.logger.setLevel(logging.DEBUG)

        # Create a queue and a QueueHandler for this process
        self.queue = multiprocessing.Queue(-1)
        self.handler = logging.handlers.QueueHandler(self.queue)
        self.logger.addHandler(self.handler)

        # Create a QueueListener to forward log messages from all processes
        # to the same set of handlers
        self.listener = logging.handlers.QueueListener(self.queue, *logging.root.handlers)
        self.listener.start()
        
        
import logging
import logging.handlers
import os

class MultiProcessLogger:

    def __init__(self, logger_name, log_file_path, log_level=logging.INFO):
        self.logger = logging.getLogger(logger_name)
        self.logger.setLevel(log_level)

        # Create a rotating file handler that can be shared between processes
        log_handler = logging.handlers.RotatingFileHandler(log_file_path, maxBytes=100000, backupCount=2)
        log_handler.setLevel(log_level)

        # Create a formatter and add it to the handler
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        log_handler.setFormatter(formatter)

        # Add the handler to the logger
        self.logger.addHandler(log_handler)

        # Ensure that logging messages are flushed immediately
        log_handler.flush()

    def log(self, message):
        self.logger.info(message)

if __name__ == '__main__':
    # Create a logger instance and log a message
    logger = MultiProcessLogger('my_logger', '/var/log/my_app.log

"""