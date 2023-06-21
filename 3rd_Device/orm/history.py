import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,BOOLEAN, SMALLINT, DATETIME, BIGINT

Base = declarative_base()

class History(Base):
   __tablename__ = 'History'
   HistoryID = Column(BIGINT,autoincrement=True,primary_key=True,nullable=False)
   Type = Column(String(length=100))
   TimeRegistered = Column(DATETIME)
   IDTarget = Column(BIGINT)

   def __repr__(self):
      attributes = vars(self)
      attribute_strings = []
      for attr, value in attributes.items():
         attribute_strings.append(f"{attr}={value}")
      return f"Client({', '.join(attribute_strings)})"