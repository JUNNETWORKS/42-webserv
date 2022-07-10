import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--WEBSERV_PORT", type=int, default=49200)

args = parser.parse_args()

WEBSERV_PORT = args.WEBSERV_PORT
