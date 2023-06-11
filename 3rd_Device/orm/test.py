import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT, ForeignKey, BIGINT, DATETIME

from orm.client import Client

Base = declarative_base()

class Test(Base):
   __tablename__ = 'Tests'
   TestID = Column(BIGINT,autoincrement=True,primary_key=True,nullable=False)
   ClientID = Column(Integer,ForeignKey(Client.ClientID), nullable=False)
   TimeTaken = Column(DATETIME)
   Status = Column(sqlalchemy.SMALLINT)

