import datetime
from unittest import TestCase
from db.packet_manager import DBPPackageManager
from db.session import SessionMaker
from orm.package import Package
from datetime import datetime

class TestDBPPackageManager(TestCase):
    def setUp(self) -> None:
        self.sesion = SessionMaker.create_test_session()
        self.db_manager = DBPPackageManager(self.sesion)

    def tearDown(self) -> None:
        self.sesion.close()

    def test_add_packager(self):
        sourceIP="127.0.0.1"
        sourcePort=23
        destIP ="192.168.1.1"
        destPort = 6744
        n_proto=3
        t_proto=2
        a_proto=3
        payload =b"hello world"
        self.db_manager.add_packager(sourceIP,sourcePort,destIP,destPort,n_proto,t_proto,a_proto,payload)
        self.assertTrue(True)
        packs = self.sesion.query(Package).all()
        for pack in packs:
            self.sesion.delete(pack)
            self.sesion.commit()

    def test_get_packages(self):
        sourceIP = "127.0.0.1"
        sourcePort = 23
        destIP = "192.168.1.1"
        destPort = 6744
        n_proto = 3
        t_proto = 2
        a_proto = 3
        payload = b"hello world"
        self.db_manager.add_packager(sourceIP, sourcePort, destIP, destPort, n_proto, t_proto, a_proto, payload)
        pack = self.sesion.query(Package).filter(Package.SourceIP == sourceIP, Package.DestinationIP == destIP).first()
        pack_mg = self.db_manager.get_package(pack.PackageID)

        self.assertEqual(pack.SourceIP, pack_mg.SourceIP)
        self.assertEqual(pack.DestinationIP, pack_mg.DestinationIP)
        self.assertEqual(pack.PackageID,pack_mg.PackageID)
        self.sesion.delete(pack)
        self.sesion.commit()


    def test_get_package_from_last(self):
        sourceIP = "127.0.0.1"
        sourcePort = 23
        destIP = "192.168.1.1"
        destPort = 6744
        n_proto = 3
        t_proto = 2
        a_proto = 3
        payload = b"hello world"
        self.db_manager.add_packager(sourceIP, sourcePort, destIP, destPort, n_proto, t_proto, a_proto, payload)
        ls = self.db_manager.get_package_from_last(datetime.utcnow())
        self.assertEqual(len(ls),1)
        for pack in ls:
            self.db_manager.delete_package(pack.PackageID)