from sqlalchemy.orm import sessionmaker,scoped_session
from sqlalchemy import create_engine

class SessionMaker():
    __USERNAME = "root"
    __PASSWORD = "hugbug3578"
    __HOST = "127.0.0.1"
    __PORT = 3306
    __DBNAME = "InfectivityDB"
    __TESTDBNAME = "TestInfectivityDB"

    __engine = create_engine("mariadb+mariadbconnector://%s:%s@%s:%s/%s" % (__USERNAME,__PASSWORD,__HOST,__PORT,__DBNAME))

    @staticmethod
    def create_session():
        if SessionMaker.__engine is None:
            SessionMaker.__engine = create_engine(
                "mariadb+mariadbconnector://%s:%s@%s:%s/%s" % (__USERNAME, __PASSWORD, __HOST, __PORT, __DBNAME))
        Session = sessionmaker()
        Session.configure(bind=SessionMaker.__engine)
        return Session()

    @staticmethod
    def create_scoped_session():
        if SessionMaker.__engine is None:
            SessionMaker.__engine = create_engine(
                "mariadb+mariadbconnector://%s:%s@%s:%s/%s" % (__USERNAME, __PASSWORD, __HOST, __PORT, __DBNAME))
        session_factory = sessionmaker(autocommit=False, autoflush=False, bind=SessionMaker.__engine, future=True)
        Session = scoped_session(session_factory)
        return Session()

    @staticmethod
    def create_test_session():
        Session = sessionmaker()
        Session.configure(bind=create_engine(
                "mariadb+mariadbconnector://%s:%s@%s:%s/%s" % (SessionMaker.__USERNAME,
                                                               SessionMaker.__PASSWORD, SessionMaker.__HOST,
                                                               SessionMaker.__PORT, SessionMaker.__TESTDBNAME)))
        return Session()
    # @staticmethod
    # def close_seesion(session:sessionmaker):
    #     session.
