from unittest import TestCase
from db.sample_manager import DBSampleManager
from db.session import SessionMaker
from orm.platform import Platform
from orm.category import Category
from orm.sample import Sample

class TestDBSampleManager(TestCase):
    def setUp(self) -> None:
        self.sesion = SessionMaker.create_test_session()
        self.db_manager = DBSampleManager(self.sesion)

    def tearDown(self) -> None:
        self.sesion.close()

    def test_add_platform_valid(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name,score)
        cl = self.sesion.query(Platform).filter(Platform.Name == name, Platform.Score == score).first()
        self.assertIsNotNone(cl)
        self.sesion.delete(cl)
        self.sesion.commit()


    def test_add_platform_twice(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name,score)
        try:
            self.db_manager.add_platform(name, score)
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Platform).filter(Platform.Name == name, Platform.Score == score).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_platform_score_invalid(self):
        name = "aaa"
        score = -3
        try:
            self.db_manager.add_platform(name, score)
            self.fail()
        except:
            self.assertTrue(True)

    def test_remove_platform(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name, score)
        self.db_manager.remove_platform(name)
        cl = self.sesion.query(Platform).filter(Platform.Name == name, Platform.Score == score).first()
        self.assertIsNone(cl)

    def test_remove_platform_inexistent(self):
        try:
            self.db_manager.remove_platform("68634387")
            self.fail()
        except:
            self.assertTrue(True)

    def test_update_platform_score(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name, score)
        self.db_manager.update_platform_score(name,9)
        cl = self.sesion.query(Platform).filter(Platform.Name == name, Platform.Score == 9).first()
        self.assertIsNotNone(cl)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_update_platform_score_inexistent(self):
        try:
            self.db_manager.update_platform_score("336486538648")
            self.fail()
        except:
            self.assertTrue(True)

    def test_update_platform_score_invalid_score(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name, score)
        try:
            self.db_manager.update_platform_score(name,-9)
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Platform).filter(Platform.Name == name, Platform.Score == score).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_platform(self):
        name = "aaa"
        score = 7
        self.db_manager.add_platform(name, score)
        cl = self.db_manager.get_platform(name)
        self.assertIsNotNone(cl)
        self.db_manager.remove_platform(name)

    def test_get_platform_inexistent(self):
        cl = self.db_manager.get_platform("etrteuer")
        self.assertIsNone(cl)

    def test_add_category_valid(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name,score)
        cl = self.sesion.query(Category).filter(Category.Name == name, Category.Score == score).first()
        self.assertIsNotNone(cl)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_category_twice(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name,score)
        try:
            self.db_manager.add_category(name, score)
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Category).filter(Category.Name == name, Category.Score == score).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_add_category_score_invalid(self):
        name = "aaa"
        score = -3
        try:
            self.db_manager.add_category(name, score)
            self.fail()
        except:
            self.assertTrue(True)

    def test_remove_category(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.remove_category(name)
        cl = self.sesion.query(Category).filter(Category.Name == name, Category.Score == score).first()
        self.assertIsNone(cl)

    def test_remove_category_inexistent(self):
        try:
            self.db_manager.remove_category("68634387")
            self.fail()
        except:
            self.assertTrue(True)

    def test_update_category_score(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.update_category_score(name,9)
        cl = self.sesion.query(Category).filter(Category.Name == name, Category.Score == 9).first()
        self.assertIsNotNone(cl)
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_update_category_score_inexistent(self):
        try:
            self.db_manager.update_category_score("336486538648")
            self.fail()
        except:
            self.assertTrue(True)

    def test_update_category_score_invalid_score(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        try:
            self.db_manager.update_category_score(name,-9)
            self.fail()
        except:
            self.assertTrue(True)
        cl = self.sesion.query(Category).filter(Category.Name == name, Category.Score == score).first()
        self.sesion.delete(cl)
        self.sesion.commit()

    def test_get_category(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        cl = self.db_manager.get_category(name)
        self.assertIsNotNone(cl)
        self.db_manager.remove_category(name)

    def test_get_platform_inexistent(self):
        cl = self.db_manager.get_category("etrteuer")
        self.assertIsNone(cl)

    def test_add_sample(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        self.db_manager.add_sample(name,name,name)
        cl = self.sesion.query(Sample).filter(Sample.Name == name).first()
        self.assertEqual(cl.Score,14)
        self.sesion.delete(cl)
        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        self.db_manager.add_sample(name, name, name,20)
        cl = self.sesion.query(Sample).filter(Sample.Name == name).first()
        self.assertEqual(cl.Score, 20)
        self.sesion.delete(cl)
        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

    def test_add_sample_invalid(self):
        name = "aaa"
        score = 7

        # no category or platform
        try:
            self.db_manager.add_sample(name, name, name)
            self.fail()
        except:
            self.assertTrue(True)


        # no platform
        self.db_manager.add_category(name, score)
        try:
            self.db_manager.add_sample(name, name, name)
            self.fail()
        except:
            self.assertTrue(True)
        self.db_manager.remove_category(name)

        # no category
        self.db_manager.add_platform(name, score)
        try:
            self.db_manager.add_sample(name, name, name)
            self.fail()
        except:
            self.assertTrue(True)
        self.db_manager.remove_platform(name)

        # invalid score
        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        try:
            self.db_manager.add_sample(name, name, name,-9)
            self.fail()
        except:
            self.assertTrue(True)

        # add twice
        self.db_manager.add_sample(name, name, name)
        try:
            self.db_manager.add_sample(name, name, name)
            self.fail()
        except:
            self.assertTrue(True)

        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

    def test_remove_sample(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        self.db_manager.add_sample(name, name, name)
        self.db_manager.remove_sample(name,name,name)
        cl = self.sesion.query(Sample).filter(Sample.Name == name).first()
        self.assertIsNone(cl)
        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

    def test_update_sample_score(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        self.db_manager.add_sample(name, name, name)
        self.db_manager.update_sample_score(name,name,name,20)
        cl = self.sesion.query(Sample).filter(Sample.Name == name).first()
        self.assertEqual(cl.Score, 20)
        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

    def test_get_sample_valid(self):
        name = "aaa"
        score = 7
        self.db_manager.add_category(name, score)
        self.db_manager.add_platform(name, score)
        self.db_manager.add_sample(name, name, name)
        cl1 =  self.db_manager.get_sample(name,name,name)
        cl = self.sesion.query(Sample).filter(Sample.Name == name).first()
        self.assertEqual(cl.SampleID, cl1.SampleID)
        self.db_manager.remove_category(name)
        self.db_manager.remove_platform(name)
        self.sesion.commit()

    def test_get_sample_invalid(self):
        name = "aaa"
        score = 1
        try:
            self.db_manager.add_sample(name, name, name)
            self.fail()
        except:
            self.assertTrue(True)

