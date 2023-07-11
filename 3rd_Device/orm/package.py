import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT, BIGINT, DATETIME, BLOB

Base = declarative_base()

class Package(Base):
   __tablename__ = 'Packages'
   PackageID = Column(BIGINT,autoincrement=True,primary_key=True,nullable=False)
   SourceIP = Column(String(length=20))
   SourcePort = Column(Integer)
   DestinationIP = Column(String(length=20))
   DestinationPort = Column(Integer)
   NetworkProtocol = Column(SMALLINT)
   TransportProtocol = Column(SMALLINT)
   ApplicationProtocol = Column(SMALLINT)
   ArriveTime = Column(DATETIME)
   PayloadSize = Column(BIGINT)
   Payload = Column(BLOB(length=500))
