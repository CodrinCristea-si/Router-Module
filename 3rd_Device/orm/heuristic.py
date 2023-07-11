import sqlalchemy
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column,Integer,String,Boolean, SMALLINT, BIGINT, DATETIME, BLOB

Base = declarative_base()

class Heuristic(Base):
   __tablename__ = 'Heuristics'
   HeuristicID = Column(BIGINT,autoincrement=True,primary_key=True,nullable=False)
   Name = Column(String(length=100))
   Type = Column(SMALLINT, default=0)
   Requirements = Column(String(length=200))
   Path = Column(String(length=200))

   def __repr__(self):
      attributes = vars(self)
      attribute_strings = []
      for attr, value in attributes.items():
         attribute_strings.append(f"{attr}={value}")
      return f"Client({', '.join(attribute_strings)})"