#!/usr/bin/python3 -u

import sys
import json
import sqlite3
import os
import cgi
import cgitb
from typing import Dict, List, Tuple
cgitb.enable()  # デバッグモードを有効にする

BLOG_DB_NAME = "/tmp/blog.db"


class BlogDB:
    def __init__(self) -> None:
        self.conn = None

    def create(self, title, body) -> int:
        cur = self.conn.cursor()
        cur.execute("""
        INSERT INTO Posts (title, body) VALUES (?, ?)
        """, (title, body))
        self.conn.commit()
        return cur.lastrowid

    def read(self, id) -> List[Tuple[int, str, str]]:
        cur = self.conn.cursor()
        posts = cur.execute(f"SELECT * FROM Posts WHERE id = {id}")
        return posts

    def read_all(self) -> List[Tuple[int, str, str]]:
        cur = self.conn.cursor()
        posts = cur.execute("SELECT * FROM Posts ORDER BY id")
        return posts

    def delete(self, id) -> None:
        cur = self.conn.cursor()
        cur.execute(f"DELETE FROM Posts WHERE id = {id}")
        self.conn.commit()

    def create_table(self):
        cur = self.conn.cursor()
        cur.execute("""
            CREATE TABLE IF NOT EXISTS Posts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT,
                body TEXT
            );
        """)
        self.conn.commit()

    def __enter__(self):
        self.conn = sqlite3.connect(BLOG_DB_NAME)
        self.create_table()
        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self.conn.close()


def get_url_params() -> Dict:
    params = dict()
    param_strs = os.environ["QUERY_STRING"].split("&")
    for param_str in param_strs:
        if param_str.find("=") != -1:
            equal_idx = param_str.find("=")
            key = param_str[:equal_idx]
            value = param_str[equal_idx + 1:]
            params[key] = value
        else:
            params[param_str] = ""
    return params


def print_header(content_type="application/json", status="200 OK", **headers):
    print(f"Content-Type: {content_type}")
    print(f"Status: {status}")
    for key, value in headers.items():
        print(f"{key}: {value}")
    print()


def generate_reponse_from_posts(posts):
    return {"posts": [{"id": post[0], "title":post[1], "body": post[2]} for post in posts]}


def print_get_response():
    url_params = get_url_params()
    with BlogDB() as db:
        if "id" in url_params:
            posts = db.read(id)
        else:
            posts = db.read_all()
        res_body = generate_reponse_from_posts(posts)

    print_header()
    print(json.dumps(res_body))


def read_all_data_from_stdin():
    content_length = int(os.environ['CONTENT_LENGTH'])
    data = sys.stdin.read(content_length)
    return data


def print_post_response():
    data = read_all_data_from_stdin()
    print(f"data: {data}")
    post = json.loads(data)
    with BlogDB() as db:
        id = db.create(post["title"], post["body"])
        posts = db.read(id)
        res_body = generate_reponse_from_posts(posts)

    print_header(status="201 Created")
    print(json.dumps(res_body))


def print_delete_response():
    url_params = get_url_params()
    if "id" not in url_params:
        print_header(status="404 Bad Request")
        return

    with BlogDB() as db:
        db.delete(url_params["id"])
    print_header()


def print_error_response():
    print_header(status="400 Bad Request")


def main():
    # cgi.test()

    method = os.environ["REQUEST_METHOD"]
    if (method == "GET"):
        print_get_response()
    elif (method == "POST"):
        print_post_response()
    elif (method == "DELETE"):
        print_delete_response()
    else:
        print_error_response()


main()
