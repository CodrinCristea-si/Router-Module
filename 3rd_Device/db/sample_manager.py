from db.session import SessionMaker
from db.db_manager import DBManager
from orm.platform import Platform
from orm.category import Category
from orm.sample import Sample

class DBSampleManager(DBManager):
    __platforms = {
        'Swf': 5,
        'Router': 10,
        'PUA': 1,
        'Win': 5,
        'Doc': 5,
        'Email': 1,
        'Symbos': 10,
        'Blocklist': 30,
        'Packed': 10,
        'Php': 5,
        'Java': 5,
        'Asp': 5,
        'Hwp': 5,
        'Xls': 5,
        'Multios': 30,
        'Phish': 10,
        'Rtf': 5,
        'Trusted': 10,
        'Ttf': 5,
        'Emf': 5,
        'Phishtank': 10,
        'Lnk': 5,
        'Svg': 5,
        'Ios': 10,
        'Dos': 10,
        'Andr': 10,
        'Archive': 30,
        'Xml': 5,
        'Mp4': 5,
        'Rat': 10,
        'Heuristics': 30,
        'Vbs': 20,
        'Clamav': 30,
        'Mkv': 5,
        'Legacy': 1,
        'Html': 5,
        'Py': 5,
        'Tool': 10,
        'Onenote': 10,
        'Gif': 5,
        'Unix': 10,
        'Revoked': 20,
        'Ps1': 30,
        'W32S': 10,
        'Js': 5,
        'Ppt': 5,
        'Pdf': 10,
        'Img': 5,
        'Ole2': 5,
        'Osx': 10,
        'Tif': 5,
        'Txt': 5
    }

    __categories = {
        'Swf':10,
        'Infostealer':10,
        'Trojan':70,
        'Win':10,
        'Email':1,
        'Doc':10,
        'Malware':100,
        'Packed':100,
        'File':50,
        'CA':10,
        'Php':10,
        'Java':10,
        'Rootkit':100,
        'Rtf':10,
        'Adware':1,
        'Ircbot':100,
        'Coinminer':1,
        'Ransomware':100,
        'Spyware':30,
        'Loader':100,
        'Worm':100,
        'Filetype':30,
        'Embedded':30,
        'Downloader':100,
        'Dropper':100,
        'Andr':10,
        'Exploit':100,
        'Proxy':5,
        'TS':10,
        'Spy':50,
        'Backdoor':100,
        'Test':1,
        'Html':10,
        'Tool':100,
        'Virus':100,
        'Macro':10,
        'Revoked':50,
        'Unix':10,
        'Js':10,
        'Keylogger':50,
        'CRT':5,
        'Countermeasure':100,
        'Pdf':30,
        'Joke':1,
        'Phishing':30,
        'Osx':10,
        'Packer':30,
        'Cert':5
        }

    def __init__(self, session: SessionMaker = None):
        # self._session = None
        super().__init__(session)

    @staticmethod
    def get_list_platforms():
        return DBSampleManager.__platforms.copy()

    @staticmethod
    def get_list_categories():
        return DBSampleManager.__categories.copy()

    def add_platform(self,name:str,score:int):
        plat = self.get_platform(name)
        if plat is not None:
            raise Exception("Platform %s already exists" % name)
        if score < 0:
            raise Exception("The score must be positive")
        plat = Platform(Name=name, Score=score)
        self._session.add(plat)
        self._session.commit()

    def remove_platform(self,name:str):
        plat = self.get_platform(name)
        if plat is None:
            raise Exception("Platform %s does not exist" % name)
        self.remove_samples_for_platform(name)
        self._session.delete(plat)
        self._session.commit()

    def update_platform_score(self,name:str,score:int):
        plat = self.get_platform(name)
        if plat is None:
            raise Exception("Platform %s does not exist" % name)
        if score < 0:
            raise Exception("The score must be positive")
        plat.Score = score
        self._session.commit()

    def get_platform(self,name:str):
        plat = self._session.query(Platform).filter(Platform.Name == name).first()
        return plat

    def get_all_platforms(self):
        plats = self._session.query(Platform).all()
        return plats
    def add_category(self,name:str,score:int):
        cat = self.get_category(name)
        if cat is not None:
            raise Exception("Category %s already exists" % name)
        if score < 0:
            raise Exception("The score must be positive")
        cat = Category(Name=name, Score=score)
        self._session.add(cat)
        self._session.commit()

    def remove_category(self,name:str):
        cat = self.get_category(name)
        if cat is None:
            raise Exception("Category %s does not exist" % name)
        self.remove_samples_for_category(name)
        self._session.delete(cat)
        self._session.commit()

    def update_category_score(self,name:str,score:int):
        cat = self.get_category(name)
        if cat is None:
            raise Exception("Category %s does not exist" % name)
        if score < 0:
            raise Exception("The score must be positive")
        cat.Score = score
        self._session.commit()

    def get_category(self,name:str):
        cat = self._session.query(Category).filter(Category.Name == name).first()
        return cat

    def get_all_categories(self):
        cats = self._session.query(Category).all()
        return cats
    def add_sample(self,platform_name:str,category_name:str,sample_name:str,score:int = -1):
        plat = self.get_platform(platform_name)
        if plat is None:
            raise Exception("Platform %s does not exist" % platform_name)
        cat = self.get_category(category_name)
        if cat is None:
            raise Exception("Category %s does not exist" %category_name)
        if score < -1:
            raise Exception("Score must be pozitive")
        sample = self.get_sample(platform_name,category_name,sample_name)
        if sample is not None:
            raise Exception("Sample %s.%s.%s already exists"%(platform_name,category_name,sample_name))
        sample = Sample(PlatformID = plat.PlatformID, CategoryID= cat.CategoryID, Name= sample_name)
        if score == -1:
            sample.Score= plat.Score + cat.Score
        else:
            sample.Score = score
        self._session.add(sample)
        self._session.commit()

    def remove_sample(self,platform_name:str,category_name:str,sample_name:str):
        plat = self.get_platform(platform_name)
        if plat is None:
            raise Exception("Platform %s does not exist" % platform_name)
        cat = self.get_category(category_name)
        if cat is None:
            raise Exception("Category %s does not exist" %category_name)
        sample = self.get_sample(platform_name,category_name,sample_name)
        if sample is None:
            raise Exception("Sample %s.%s.%s does not exist"%(platform_name,category_name,sample_name))
        self._session.delete(sample)
        self._session.commit()

    def remove_samples_for_platform(self,platform_name:str):
        plat = self.get_platform(platform_name)
        if plat is None:
            raise Exception("Platform %s does not exist" % platform_name)
        samples = self._session.query(Sample).filter(Sample.PlatformID == plat.PlatformID).all()
        for sample in samples:
            self._session.delete(sample)
            self._session.commit()

    def remove_samples_for_category(self,category_name:str):
        cat = self.get_category(category_name)
        if cat is None:
            raise Exception("Category %s does not exist" % category_name)
        samples = self._session.query(Sample).filter(Sample.CategoryID == cat.CategoryID).all()
        for sample in samples:
            self._session.delete(sample)
            self._session.commit()

    def update_sample_score(self,platform_name:str,category_name:str,sample_name:str,score:int):
        plat = self.get_platform(platform_name)
        if plat is None:
            raise Exception("Platform %s does not exist" % platform_name)
        cat = self.get_category(category_name)
        if cat is None:
            raise Exception("Category %s does not exist" %category_name)
        if score < 0:
            raise Exception("Score must be pozitive")
        sample = self.get_sample(platform_name,category_name,sample_name)
        if sample is None:
            raise Exception("Sample %s.%s.%s does not exist"%(platform_name,category_name,sample_name))
        sample.Score= score
        self._session.commit()

    def get_all_samples(self):
        samples = self._session.query(Sample).all()
        return samples

    def get_sample(self, platform_name:str,category_name:str,sample_name:str):
        plat = self.get_platform(platform_name)
        if plat is None:
            return None
        cat = self.get_category(category_name)
        if cat is None:
            return None
        samp = self._session.query(Sample).filter(Sample.PlatformID == plat.PlatformID, Sample.CategoryID==cat.CategoryID,Sample.Name==sample_name).first()
        return samp

    def get_samples_stats_by_platforms(self):
        plats = self._session.query(Platform).all()
        samps = {}
        for plat in plats:
            nr_samps = self._session.query(Sample).filter(Sample.PlatformID == plat.PlatformID).count()
            samps[plat.Name] = nr_samps
        return samps

    def get_samples_stats_by_categories(self):
        cats = self._session.query(Category).all()
        samps = {}
        for cat in cats:
            nr_samps = self._session.query(Sample).filter(Sample.CategoryID==cat.CategoryID).count()
            samps[cat.Name] = nr_samps
        return samps
