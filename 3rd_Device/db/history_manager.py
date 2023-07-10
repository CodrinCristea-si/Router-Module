from db.session import SessionMaker
from db.db_manager import DBManager
from orm.history import History
from datetime import datetime, timedelta
from sqlalchemy import delete

class DBHistoryManager(DBManager):
    def __init__(self, session:SessionMaker= None):
        #self.__session = None
        super().__init__(session)

    def add_history(self,type_desc,id_target):
        hist = History(Type = type_desc, TimeRegistered = datetime.now(), IDTarget = id_target)
        self._session.add(hist)
        self._session.commit()

    def get_last_nr_history(self, nr_hist):
        ls_hists = []
        if nr_hist > 0:
            ls_hists = self._session.query(History).order_by(History.HistoryID.desc()).limit(nr_hist).all()
        else:
            ls_hists = self._session.query(History).order_by(History.HistoryID.desc()).all()
        return ls_hists

    def delete_all_history(self):
        delete_statement = delete(History)
        self._session.execute(delete_statement)
        self._session.commit()
