import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT

Base = declarative_base()

class Platform(Base):
   __tablename__ = 'Platforms'
   PlatformID = Column(SMALLINT,autoincrement=True,primary_key=True,nullable=False)
   Name = Column(String(length=20))
   Score = Column(sqlalchemy.SMALLINT)

