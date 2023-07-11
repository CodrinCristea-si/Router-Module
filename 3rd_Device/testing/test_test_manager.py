from unittest import TestCase
from db.test_manager import DBTestManagement
from db.client_manager import DBClientManager
from db.sample_manager import DBSampleManager
from db.session import SessionMaker

from orm.client import Client
from orm.test import Test
from orm.sample import Sample
from orm.test_status import TestStatus


class TestDBTestManagement(TestCase):
    def setUp(self) -> None:
        self.sesion = SessionMaker.create_test_session()
        self.test_db_manager = DBTestManagement(self.sesion)
        self.client_db_manager = DBClientManager(self.sesion)
        self.sample_db_manager = DBSampleManager(self.sesion)

    def tearDown(self) -> None:
        self.sesion.close()
    def test_add_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()
        self.assertIsNotNone(test)
        self.assertEqual(test.Status, TestStatus.STARTED.value)
        self.sesion.delete(test)
        self.sesion.commit()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_test_inexistent_client(self):
        try:
            self.test_db_manager.add_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)


    def test_add_test_twice(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        try:
            self.test_db_manager.add_test(ip, mac)
            self.fail()
        except:
            self.assertTrue(True)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_begin_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        self.test_db_manager.begin_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()
        self.assertEqual(test.Status, TestStatus.RUNNING.value)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_begin_test_inexistent_client(self):
        try:
            self.test_db_manager.begin_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)

    def test_begin_test_inexistent_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        try:
            self.test_db_manager.begin_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_finish_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        self.test_db_manager.begin_test(ip, mac)
        self.test_db_manager.finish_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()
        self.assertEqual(test.Status, TestStatus.FINISHED.value)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_finish_test_inexistent_client(self):
        try:
            self.test_db_manager.finish_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)

    def test_finish_test_inexistent_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        try:
            self.test_db_manager.finish_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_finish_test_not_running(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        try:
            self.test_db_manager.finish_test("ip","mac")
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_test_from_last(self):
        ip1 = "192.168.1.1"
        mac1 = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip1, mac1)
        self.test_db_manager.add_test(ip1, mac1)

        ip2 = "192.168.1.2"
        mac2 = "aa:bb:cc:ee:ee:ff"
        self.client_db_manager.add_client(ip2, mac2)
        self.test_db_manager.add_test(ip2, mac2)

        tests= self.test_db_manager.get_test_from_last(5)
        self.assertEqual(len(tests),2)

        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip1, Client.MAC == mac1).first()
        self.sesion.delete(cl)
        self.sesion.commit()
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip2, Client.MAC == mac2).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_last_started_test_for_client(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        test1= self.test_db_manager.get_last_started_test_for_client(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()
        self.assertEqual(test.TestID,test1.TestID)
        self.assertEqual(test1.Status, TestStatus.STARTED.value)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_last_running_test_for_client(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        self.test_db_manager.begin_test(ip,mac)
        test1 = self.test_db_manager.get_last_running_test_for_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()
        self.assertEqual(test.TestID, test1.TestID)
        self.assertEqual(test1.Status, TestStatus.RUNNING.value)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_test_results(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        self.test_db_manager.begin_test(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()

        p1 = "a1"
        c1 = "b1"
        n1 = "c1"
        self.sample_db_manager.add_platform(p1,1)
        self.sample_db_manager.add_category(c1,1)
        self.sample_db_manager.add_sample(p1,c1,n1)

        p2 = "a2"
        c2 = "b2"
        n2 = "c2"

        self.test_db_manager.add_test_results(ip,mac,[(p1,c1,n1),(p2,c2,n2)])
        samp = self.sample_db_manager.get_sample(p2,c2,n2)
        self.assertIsNotNone(samp)
        self.assertEqual(samp.Score,20)

        ls = self.test_db_manager.get_last_test_results(ip,mac)
        self.assertEqual(len(ls), 2)

        self.sample_db_manager.remove_platform(p1)
        self.sample_db_manager.remove_platform(p2)
        self.sample_db_manager.remove_category(c1)
        self.sample_db_manager.remove_category(c2)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_test_results_not_running(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.client_db_manager.add_client(ip, mac)
        self.test_db_manager.add_test(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        test = self.sesion.query(Test).filter(Test.ClientID == cl.ClientID).first()

        p1 = "a1"
        c1 = "b1"
        n1 = "c1"
        self.sample_db_manager.add_platform(p1, 1)
        self.sample_db_manager.add_category(c1, 1)
        self.sample_db_manager.add_sample(p1, c1, n1)

        p2 = "a2"
        c2 = "b2"
        n2 = "c2"

        try:
            self.test_db_manager.add_test_results(ip, mac, [(p1, c1, n1), (p2, c2, n2)])
            self.fail()
        except:
            self.assertTrue(True)

        self.sample_db_manager.remove_platform(p1)
        self.sample_db_manager.remove_category(c1)
        self.sesion.delete(cl)
        self.sesion.commit()