from db.session import SessionMaker
from db.db_manager import DBManager
from orm.package import Package
from datetime import datetime

class DBPPackageManager(DBManager):
    def __init__(self, session: SessionMaker = None):
        # self._session = None
        super().__init__(session)

    def add_packager(self,sourceIP:str,sourcePort:int,destIP:str,destPort:int,n_proto:int,t_proto:int,a_proto:int, payload:bytes):
        pack = Package(SourceIP= sourceIP,SourcePort=sourcePort,DestinationIP=destIP,DestinationPort=destPort, NetworkProtocol=n_proto,TransportProtocol=t_proto,ApplicationProtocol=a_proto, ArriveTime=datetime.utcnow(),Payload=payload)
        self._session.add(pack)
        self._session.commit()

    def get_package(self,id):
        pack = self._session.query(Package).filter(Package.PackageID == id).first()
        return pack

    def get_package_from_last(self,time):
        packs = self._session.query(Package).filter(datetime.utcnow() - Package.ArriveTime < time).all()
        return packs

    def delete_package(self,id:int):
        pack = self._session.query(Package).filter(Package.PackageID == id ).first()
        self._session.delete(pack)
        self._session.commit()