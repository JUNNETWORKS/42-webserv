import sys

GREEN = ""
RED = ""
RESET = ""
if sys.stdout.isatty():
    GREEN = "\033[32m"
    RED = "\033[31m"
    RESET = "\033[39m"
