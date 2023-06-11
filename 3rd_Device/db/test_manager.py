from db.session import SessionMaker
from db.db_manager import DBManager
from datetime import datetime
from orm.test_status import TestStatus
from orm.test import Test
from orm.test_results import TestResult
from orm.sample import Sample

from db.sample_manager import DBSampleManager
from db.client_manager import DBClientManager

class DBTestManagement(DBManager):
    def __init__(self, session: SessionMaker = None):
        # self._session = None
        super().__init__(session)
        self.__sample_manager = DBSampleManager(self._session)
        self.__client_manager = DBClientManager(self._session)

    def add_test(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip,mac)
        if cl is None:
            raise Exception("Client with ip %s and mac %s does not exist" %(ip,mac))
        test = self.get_last_started_test_for_client(ip,mac)
        if test is not None:
            raise Exception("There is a test started for the %s->%s"%(mac,ip))
        test = Test(ClientID = cl.ClientID,TimeTaken = datetime.utcnow(),Status= TestStatus.STARTED.value)
        self._session.add(test)
        self._session.commit()

    def begin_test(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip,mac)
        if cl is None:
            raise Exception("Client with ip %s and mac %s does not exist" %(ip,mac))
        test = self._session.query(Test).filter(Test.ClientID == cl.ClientID,Test.Status== TestStatus.STARTED.value).order_by(Test.TestID.desc()).first()
        if test is None:
            raise Exception("There are no running tests for %s->%s"%(mac,ip))
        test.Status = TestStatus.RUNNING.value
        self._session.commit()

    def finish_test(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip,mac)
        if cl is None:
            raise Exception("Client with ip %s and mac %s does not exist" %(ip,mac))
        test = self._session.query(Test).filter(Test.ClientID == cl.ClientID,Test.Status== TestStatus.RUNNING.value).order_by(Test.TestID.desc()).first()
        if test is None:
            raise Exception("There are no running tests for %s->%s"%(mac,ip))
        test.Status = TestStatus.FINISHED.value
        self._session.commit()

    def get_test_from_last(self,time):
        tests = self._session.query(Test).filter(datetime.utcnow() - Test.TimeTaken < time).all()
        return tests

    def get_last_started_test_for_client(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip,mac)
        if cl is None:
            raise Exception("Client %s->%s does not exist" %(mac,ip))
        test = self._session.query(Test).filter(Test.ClientID == cl.ClientID, Test.Status == TestStatus.STARTED.value).order_by(Test.TestID.desc()).first()
        return test

    def get_last_running_test_for_client(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip,mac)
        if cl is None:
            raise Exception("Client %s->%s does not exist" %(mac,ip))
        test = self._session.query(Test).filter(Test.ClientID == cl.ClientID, Test.Status == TestStatus.RUNNING.value).order_by(Test.TestID.desc()).first()
        return test

    def get_last_test_for_client(self,ip:str,mac:str):
        cl = self.__client_manager.get_client(ip, mac)
        if cl is None:
            raise Exception("Client %s->%s does not exist" % (mac, ip))
        test = self._session.query(Test).filter(Test.ClientID == cl.ClientID).order_by(Test.TestID.desc()).first()
        return test

    def add_test_results(self,ip:str, mac:str, samples:list):
        """
        :param samples: [(platform,category,name),...]
        """
        test = self.get_last_running_test_for_client(ip,mac)
        if test is None:
            raise Exception("Thre is no test for %s->%s"%(mac,ip))
        for sample in samples:
            samp = self.__sample_manager.get_sample(sample[0],sample[1],sample[2])
            if samp is None:
                self.__sample_manager.add_platform(sample[0],10)
                self.__sample_manager.add_category(sample[1],10)
                self.__sample_manager.add_sample(sample[0],sample[1],sample[2])
                samp = self.__sample_manager.get_sample(sample[0], sample[1], sample[2])
            test_res = self._session.query(TestResult).fiter(TestResult.TestID == test.TestID, TestResult.SampleID== samp.SampleID).first()
            if test_res is None:
                test_res = TestResult(TestID= test.TestID, SampleID= samp.SampleID, NumberOfTimes = 1)
                self._session.add(test_res)
            else:
                test_res.NumberOfTimes += 1
            self._session.commit()

    def get_last_test_results(self,ip:str,mac:str):
        test = self.get_last_test_for_client(ip,mac)
        lista = self._session.query(TestResult).filter(TestResult.TestID==test.TestID).all()
        ls_samp = [self._session.query(Sample).filter(Sample.SampleID == tr.SampleID).first() for tr in lista ]
        return ls_samp
