from db.session import SessionMaker


class DBManager:
    def __init__(self, session: SessionMaker = None):
        if session is None:
            self._session = SessionMaker.create_session()
        else:
            self._session = session