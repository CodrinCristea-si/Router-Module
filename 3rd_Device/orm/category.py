import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT

Base = declarative_base()

class Category(Base):
   __tablename__ = 'Categories'
   CategoryID = Column(SMALLINT,autoincrement=True,primary_key=True,nullable=False)
   Name = Column(String(length=20))
   Score = Column(sqlalchemy.SMALLINT)

