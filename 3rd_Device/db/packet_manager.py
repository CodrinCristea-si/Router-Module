from db.session import SessionMaker
from db.db_manager import DBManager
from orm.package import Package
from datetime import datetime, timedelta

class DBPPackageManager(DBManager):
    def __init__(self, session: SessionMaker = None):
        # self._session = None
        super().__init__(session)

    def add_packager(self,sourceIP:str,sourcePort:int,destIP:str,destPort:int,n_proto:int,t_proto:int,a_proto:int, payload_size:int, payload:bytes):
        pack = Package(SourceIP= sourceIP,SourcePort=sourcePort,DestinationIP=destIP,DestinationPort=destPort, NetworkProtocol=n_proto,TransportProtocol=t_proto,ApplicationProtocol=a_proto, ArriveTime=datetime.utcnow(),PayloadSize=payload_size, Payload=payload)
        self._session.add(pack)
        self._session.commit()

    def get_package(self,id):
        pack = self._session.query(Package).filter(Package.PackageID == id).first()
        return pack

    def get_package_from_last(self,time):
        threshold_time = datetime.utcnow() - timedelta(minutes=time)
        packs = self._session.query(Package).filter(Package.ArriveTime > threshold_time).all()
        if packs is None:
            return []
        return packs

    def delete_package(self,id:int):
        pack = self._session.query(Package).filter(Package.PackageID == id ).first()
        self._session.delete(pack)
        self._session.commit()

    def get_last_minutes_packages(self,ip:str,nr_min:int):
        last_pack = self._session.query(Package).filter(Package.SourceIP == ip).order_by(Package.PackageID.desc()).first()
        if last_pack is None:
            return []
        threshold_time = last_pack.ArriveTime - timedelta(minutes=nr_min)
        ls_packs_s = self._session.query(Package).filter(Package.SourceIP == ip, Package.ArriveTime > threshold_time).all()
        ls_packs_d = self._session.query(Package).filter(Package.DestinationIP == ip,
                                                         Package.ArriveTime > threshold_time).all()
        if ls_packs_s is None and ls_packs_d is None:
            return []
        if ls_packs_s is None:
            return ls_packs_d
        if ls_packs_d is None:
            return ls_packs_s

        return ls_packs_s + ls_packs_d

    def get_last_nr_packages(self,ip:str,nr_packs:int):
        ls_packs_s = self._session.query(Package).filter(Package.SourceIP == ip).order_by(Package.PackageID.desc()).limit(nr_packs).all()
        ls_packs_d = self._session.query(Package).filter(Package.DestinationIP == ip).order_by(Package.PackageID.desc()).limit(nr_packs).all()

        if ls_packs_s is None and ls_packs_d is None:
            return []
        if ls_packs_s is None:
            return ls_packs_d
        if ls_packs_d is None:
            return ls_packs_s

        return ls_packs_s + ls_packs_d