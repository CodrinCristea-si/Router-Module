from db.session import *
from client import *

if __name__ == "__main__":
    session = SessionMaker.create_session()
    cl = Client(CurrentIP="192.168.1.2",MAC="a8:78:39:de:45:9f",IsConnected=True,InfectivityType=2,Score=10)
    session.add(cl)
    session.commit()
    print("Added")
    cl_n = session.query(Client).all()
    print(cl_n)

    session.delete(cl)
    session.commit()
    print("deleted")
