from ui.app import *

if __name__ == '__main__':
    app.config['MIME_TYPES'] = {'text/css': ['css']}
    app.run()