import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT, ForeignKey

from orm.platform import Platform
from orm.category import Category

Base = declarative_base()

class Sample(Base):
   __tablename__ = 'Samples'
   SampleID = Column(Integer,autoincrement=True,primary_key=True,nullable=False)
   PlatformID = Column(SMALLINT,ForeignKey(Platform.PlatformID), nullable=False)
   CategoryID = Column(SMALLINT, ForeignKey(Category.CategoryID), nullable=False)
   Name = Column(String(length=20))
   Score = Column(sqlalchemy.SMALLINT)

