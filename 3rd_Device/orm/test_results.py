import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT, ForeignKey, BIGINT

from orm.test import Test
from orm.sample import Sample

Base = declarative_base()

class TestResult(Base):
   __tablename__ = 'TestResults'
   TestID = Column(BIGINT, ForeignKey(Test.TestID),primary_key=True, nullable=False)
   SampleID = Column(Integer,ForeignKey(Sample.SampleID),primary_key=True,nullable=False)
   NumberOfTimes = Column(Integer)

