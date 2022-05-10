#!/usr/bin/python3
# https://flask.palletsprojects.com/en/2.1.x/deploying/cgi/

# 使用例
# REQUEST_METHOD=GET QUERY_STRING='hoge&fuga=2%26' ./socket_and_cgi/flask.cgi

from wsgiref.handlers import CGIHandler
from flaskapp.app import app

CGIHandler().run(app)
