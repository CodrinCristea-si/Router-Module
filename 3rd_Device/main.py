import os
from network.router_server import RouterServer
from logger.logger import Logger
from infectivity.infectivity_server import InfectivityServer
from network.client_tester import ClientTester
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from threading import Thread
samples = {}



platforms = {
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

categories = {
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

# < 20 minor
# > 100 sever
def parse_signiture_file(filename_db:str):
    with open(filename_db,"r") as file:
        line = file.readline()
        while line != "":
            data = line.split("-")[0] # skip ip and revision code
            #print(data)
            details = data.split(".")
            if details[0] == "BC":
                details = details[1:]
            if len(details) > 3:
                platform, category, name, _ = details
            elif len(details) == 3:
                platform, category, name = details

            if platform is not None and samples.get(platform) is None:
                samples[platform] = {}
            if category is not None and samples[platform].get(category) is None:
                samples[platform][category] = set()
            if name is not None:
                if not name.isnumeric():
                    details = name.split("_")
                    if len(details) == 1:
                        samples[platform][category].add(name)
                    else:
                        if len(details) == 2:
                            acro,type = name.split("_")
                        elif len(details) == 3:
                            acro, type, _ = name.split("_")
                        elif len(details) == 4:
                            acro, type, _, _ = name.split("_")
                        if acro.isnumeric():
                            samples[platform][category].add("~")
                        elif type != "ID":
                            samples[platform][category].add(acro+"_"+type)
                        else:
                            samples[platform][category].add(acro)
                else:
                    samples[platform][category].add("~")
            line = file.readline()

def save_samples(filename_out:str):
    categ = set()
    plat = set()
    with open(filename_out, "w") as file:

        for platform in samples:
            print(platform, ":",len(samples[platform]))
            plat.add(platform)
            for category in samples[platform]:
                categ.add(category)
                print(category, end=" ")
                if len(samples[platform][category]) != 0:
                    for name in samples[platform][category]:
                        score = platforms[platform] + categories[category]
                        file.write(platform + "," + category + "," + name + "," + str(score) +"\n")
            print()
        print("Nr categories:", len(categ), categ)
        print ("Nr platforms:", len(samples), plat)



def start_server(serv):
    serv.run_server()

if __name__ == "__main__":
    #parse_signiture_file("data/signature.db")
    #save_samples("data/sample.db")
    logger = Logger()

    tester = ClientTester("192.168.1.2",ITC._TESTER_PORT,logger)
    router = RouterServer("192.168.1.2",5005,logger)
    manager = InfectivityServer("127.0.0.1",5004,logger)
    p1 = Thread(target=start_server, args=(router,))
    p2 = Thread(target=start_server, args=(manager,))
    p3 = Thread(target=start_server, args=(tester,))
    p2.start()
    p3.start()
    p1.start()

    p1.join()
    p3.join()
    p2.join()
