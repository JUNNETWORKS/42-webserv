import argparse
import sys

from . import send_req_utils
from . import diff_utils
from . import io_utils

GREEN = ""
RED = ""
RESET = ""
if sys.stdout.isatty():
    GREEN = "\033[32m"
    RED = "\033[31m"
    RESET = "\033[39m"

OK_MSG = GREEN + "[ OK ]" + RESET
KO_MSG = RED + "[ KO ]" + RESET


# prm
# ========================================================================
class prms:
    def __init__(self, code=0, body="", file_path="") -> None:
        self.code = code
        self.set_body(body=body, file_path=file_path)

    def set_body(self, body, file_path):
        if body != "" and file_path != "":
            print("body or file_paht", body, file_path)
            exit(1)
        if file_path != "":
            self.body = io_utils.get_file_data(file_path)
        else:
            self.body = body


def is_eq_prms(prm1, prm2, ck_code=True, ck_body=True) -> bool:
    is_code_ok = True
    is_body_ok = True
    if ck_code:
        is_code_ok = prm1.code == prm2.code
    if ck_body:
        is_body_ok = prm1.body == prm2.body
    return is_code_ok and is_body_ok


def print_prms(prm: prms) -> None:
    print(f"prm : {prm.code}, {prm.body}")


# print
# ========================================================================
def print_func_name(f):
    print("\n---", f.__name__.upper(), "---\n")


# run Test
# ========================================================================
def run_test(req_path, expect_prm: prms, port=49200, ck_code=True, ck_body=True):
    code, body = send_req_utils.send_req(req_path, port)
    ft_res_prm = prms(code=code, body=body)

    is_success = None
    if is_eq_prms(ft_res_prm, expect_prm, ck_code=ck_code, ck_body=ck_body):
        is_success = True
        print(OK_MSG)
    else:
        is_success = False
        print(KO_MSG)
    return is_success


# launcher Test
# ========================================================================
def simple_test():
    req_path = "/sample.html"
    file_path = "public" + req_path
    expect_prm = prms(200, file_path=file_path)
    run_test(req_path, expect_prm)

    req_path = "/hoge/hoge.html"
    file_path = "public" + req_path
    expect_prm = prms(200, file_path=file_path)
    run_test(req_path, expect_prm)

    req_path = "/fuga/fuga.html"
    file_path = "public" + req_path
    expect_prm = prms(200, file_path=file_path)
    run_test(req_path, expect_prm)


def not_found_test():
    expect_prm = prms(404)
    run_test("/NotExist", expect_prm, ck_body=False)


def run_all_test():
    print_func_name(simple_test)
    simple_test()

    print_func_name(not_found_test)
    not_found_test()


# main
# ========================================================================
def main():
    run_all_test()
    diff_utils.save_diff_html()


if __name__ == "__main__":
    main()
