

class ScanResults:

    def __init__(self, list_malware:dict, ratio:list):
        self.__malware = list_malware
        self.__ratio = ratio

    @property
    def list_malware(self):
        return self.__malware

    @property
    def ratio(self):
        return self.__ratio

    def __str__(self):
        return f'Results %s with ratio %s' % (self.__malware, self.__ratio)