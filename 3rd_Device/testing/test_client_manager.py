from unittest import TestCase
from db.session import SessionMaker
from db.client_manager import DBClientManager
from orm.client import Client

class TestDBClientManager(TestCase):
    def setUp(self) -> None:
        self.sesion = SessionMaker.create_test_session()
        self.db_manager = DBClientManager(self.sesion)

    def tearDown(self) -> None:
        self.sesion.close()

    def test_add_client_new(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.IsConnected, 1)
        self.assertEqual(cl.MAC, mac)
        self.assertEqual(cl.CurrentIP, ip)
        self.assertEqual(cl.InfectivityType, 2)
        self.assertEqual(cl.Score, 0)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_client_existent(self):
        ip1 = "192.168.1.1"
        ip2 = "192.168.1.2"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip1, mac)
        self.db_manager.add_client(ip2, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip2, Client.MAC == mac).first()
        self.assertEqual(cl.IsConnected, 1)
        self.assertEqual(cl.MAC, mac)
        self.assertEqual(cl.CurrentIP, ip2)
        self.assertEqual(cl.InfectivityType, 2)
        self.assertEqual(cl.Score, 0)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_begin_client_test_valid(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        self.db_manager.begin_client_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.IsTesting,1)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_begin_client_test_disconnected(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)

        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        cl.IsConnected=0
        self.sesion.commit()

        try:
            self.db_manager.begin_client_test(ip, mac)
            self.assertTrue(False)
        except:
            self.assertTrue(True)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_begin_client_test_already_testing(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        cl.IsTesting=1
        self.sesion.commit()
        try:
            self.db_manager.begin_client_test(ip, mac)
            self.assertTrue(False)
        except:
            self.assertTrue(True)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_end_client_test(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        self.db_manager.begin_client_test(ip, mac)
        self.db_manager.end_client_test(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.IsTesting, 0)

        self.sesion.delete(cl)
        self.sesion.commit()

    def test_end_client_test_disconnected(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        cl.IsConnected = 0
        self.sesion.commit()
        try:
            self.db_manager.end_client_test(ip, mac)
            self.fail()
        except:
            self.assertTrue(True)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_end_client_test_not_testing(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        self.db_manager.begin_client_test(ip, mac)
        self.db_manager.end_client_test(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        try:
            self.db_manager.end_client_test(ip, mac)
            self.fail()
        except:
            self.assertTrue(True)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_remove_client(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        self.db_manager.remove_client(ip,mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.IsConnected,0)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_remove_client_inexistent(self):
        try:
            self.db_manager.remove_client("235.32278","3487874684")
            self.fail()
        except:
            self.assertTrue(True)

    def test_transfer_client(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        self.db_manager.transfer_client(ip,mac,3)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.InfectivityType, 3)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_transfer_client_inexistent(self):
        try:
            self.db_manager.transfer_client(ip,mac,3)
            self.fail()
        except:
            self.assertTrue(True)

    def test_transfer_client_disconnected(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        cl.IsConnected=0
        self.sesion.commit()
        try:
            self.db_manager.transfer_client(ip,mac,3)
            self.fail()
        except:
            self.assertTrue(True)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_transfer_client_invalid_type(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        try:
            self.db_manager.transfer_client(ip,mac,6)
            self.fail()
        except:
            self.assertTrue(True)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_client(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl_get = self.db_manager.get_client(ip, mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.ClientID, cl_get.ClientID)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_client_for_multiple(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)

        ip2 = "192.168.1.3"
        mac2 = "aa:bb:cc:ff:ee:ff"
        self.db_manager.add_client(ip2, mac2)
        cl_get = self.db_manager.get_client(ip2, mac2)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip2, Client.MAC == mac2).first()
        self.assertEqual(cl.ClientID, cl_get.ClientID)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_client_with_type(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        type = 2
        self.db_manager.add_client(ip, mac)
        cl_get = self.db_manager.get_client(ip, mac, type)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertEqual(cl.ClientID, cl_get.ClientID)
        self.assertEqual(cl.InfectivityType, cl_get.InfectivityType)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_client_inexistent(self):
        ip = "192.168.1.1"
        mac = "aa:bb:cc:dd:ee:ff"
        self.db_manager.add_client(ip, mac)
        cl_get = self.db_manager.get_client("192.168.1.3", mac)
        cl = self.sesion.query(Client).filter(Client.CurrentIP == ip, Client.MAC == mac).first()
        self.assertIsNone(cl_get)
        self.sesion.delete(cl)
        self.sesion.commit()