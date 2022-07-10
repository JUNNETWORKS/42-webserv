import sys
import argparse

GREEN = ""
RED = ""
RESET = ""
if sys.stdout.isatty():
    GREEN = "\033[32m"
    RED = "\033[31m"
    RESET = "\033[39m"

parser = argparse.ArgumentParser()
parser.add_argument("--WEBSERV_PORT", type=int, default=49200)

args = parser.parse_args()

WEBSERV_PORT = args.WEBSERV_PORT
