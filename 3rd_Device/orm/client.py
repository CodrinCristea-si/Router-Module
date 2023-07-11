import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,BOOLEAN, SMALLINT, DATETIME

Base = declarative_base()

class Client(Base):
   __tablename__ = 'Clients'
   ClientID = Column(Integer,autoincrement=True,primary_key=True,nullable=False)
   CurrentIP = Column(String(length=20))
   MAC = Column(String(length=20))
   IsConnected = Column(SMALLINT,default=0)
   IsTesting = Column(SMALLINT,default=0)
   InfectivityType = Column(sqlalchemy.SMALLINT)
   Score = Column(sqlalchemy.SMALLINT)
   LastConnected = Column(DATETIME)
   def __repr__(self):
      attributes = vars(self)
      attribute_strings = []
      for attr, value in attributes.items():
         attribute_strings.append(f"{attr}={value}")
      return f"Client({', '.join(attribute_strings)})"