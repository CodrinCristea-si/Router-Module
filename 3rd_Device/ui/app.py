from flask import Flask, render_template, request, url_for
from flask_cors import CORS
import orm
from infectivity.infectivity_type import InfectivityTypes
from orm.client import Client
from communicators.infectivity_tester_communicator import InfectivityTesterCommunicator as ITC
from packages.infectivity_request import *
from packages.infectivity_response import *
from db import *
from logger.logger import Logger

app = Flask(__name__, static_url_path='/static', static_folder='static')
CORS(app)

@app.route('/')
def index():
    #url_for('static', filename='css/main-page.css')
    return render_template('index.html')

