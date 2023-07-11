import os

from db.sample_manager import DBSampleManager

if __name__ == "__main__":
    db_sample = DBSampleManager()
    list_platforms = DBSampleManager.get_list_platforms()
    list_categories = DBSampleManager.get_list_categories()
    for plat in list_platforms:
        db_sample.add_platform(plat, list_platforms.get(plat))
    print("Platforms added")
    for cat in list_categories:
        db_sample.add_category(cat, list_categories.get(cat))
    print("Categories added")
    nr_ent = 0
    with open(os.getcwd() + "/../data/sample.db","r") as file:
        data = file.readline()
        while data != "" and data is not None:
            nr_ent += 1
            sample = data.split(",")
            try:
                db_sample.add_sample(sample[0],sample[1],sample[2],int(sample[3]))
            except Exception as e:
                if not "Sample" in str(e):
                    print(e)
            if nr_ent %1000 == 0:
                print("State : ", nr_ent);

            data = file.readline()
    print("Samples added")