from db.session import SessionMaker
from db.db_manager import DBManager
from domain.heuristic import Heuristic as HeuristicDOM
from orm.heuristic import Heuristic

class DBHeuristicManagement(DBManager):

    def __init__(self, session:SessionMaker= None):
        #self.__session = None
        super().__init__(session)

    def add_heuristics_dom(self,heur:HeuristicDOM):
        heur_db = self.get_heuristic(heur.name)
        if heur_db is not None:
            raise Exception("Hueristics with name %s already exists" %(heur.name))
        heur_db = Heuristic(Name=heur.name, Type=heur.type, Path=heur.path, Requirements="".join([str(el)+" " for el in heur.requirements]))
        self._session.add(heur_db)
        self._session.commit()

    def remove_heuristics(self,name:str):
        heur_db = self.get_heuristic(name)
        if heur_db is None:
            raise Exception("Hueristics with name %s does not exists" % (name))
        self._session.delete(heur_db)
        self._session.commit()

    def get_heuristic(self,name:str):
        heur = self._session.query(Heuristic).filter(Heuristic.Name == name).first()
        return heur

    def get_all_heuristics(self):
        ls_heur = self._session.query(Heuristic).all()
        return ls_heur
