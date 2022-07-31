import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--WEBSERV_PORT", type=int, default=49200)
parser.add_argument("--NGINX_PORT", type=int, default=49201)
parser.add_argument("--APACHE_PORT", type=int, default=49202)
parser.add_argument("--RUN_CMP_TEST", type=int, default=True)

args = parser.parse_args()

WEBSERV_PORT = args.WEBSERV_PORT
NGINX_PORT = args.NGINX_PORT
APACHE_PORT = args.APACHE_PORT

RUN_CMP_TEST = args.RUN_CMP_TEST
