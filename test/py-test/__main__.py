import argparse

from . import const_str
from . import send_req_utils
from . import diff_utils
from . import io_utils

parser = argparse.ArgumentParser()
parser.add_argument("--WEBSERV_PORT", type=int, default=49200)

args = parser.parse_args()

WEBSERV_PORT = args.WEBSERV_PORT

OK_MSG = const_str.GREEN + "[ OK ]" + const_str.RESET
KO_MSG = const_str.RED + "[ KO ]" + const_str.RESET

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


# KO
# ========================================================================
ko_lst = []


def append_ko_lst(case):
    ko_lst.append(case)


def print_ko_lst():
    print("\n\n\n --- KO_LST ---\n")
    print(f"KO COUNT : {len(ko_lst)}")
    print()
    for node in ko_lst:
        print(node)


# run Test
# ========================================================================
def run_test(
    req_path, expect_prm: prms, port=WEBSERV_PORT, ck_code=True, ck_body=True
) -> bool:
    code, body = send_req_utils.send_req(req_path, port)
    ft_res_prm = prms(code=code, body=body)

    is_success = None
    log_msg = f"req_path : {req_path}"
    if is_eq_prms(ft_res_prm, expect_prm, ck_code=ck_code, ck_body=ck_body):
        is_success = True
        print(OK_MSG, log_msg)
    else:
        is_success = False
        print(KO_MSG, log_msg)
        append_ko_lst([log_msg])
        diff_utils.make_diff_html(ft_res_prm.body, expect_prm.body)
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
    run_test("/NotExist/NotExist", expect_prm, ck_body=False)
    run_test("/hoge/NotExist", expect_prm, ck_body=False)


def autoindex_test():
    req_path = "/"
    file_path = "public" + "/expect_autoindex.html"
    expect_prm = prms(200, file_path=file_path)
    run_test(req_path, expect_prm)


def path_normaliz_test():
    expect_prm = prms(200, file_path="public/sample.html")
    run_test("///sample.html", expect_prm)
    run_test("/./././sample.html", expect_prm)
    run_test("/NotExist/../sample.html", expect_prm)


def run_all_test():
    print_func_name(simple_test)
    simple_test()

    print_func_name(not_found_test)
    not_found_test()

    print_func_name(autoindex_test)
    autoindex_test()

    print_func_name(path_normaliz_test)
    path_normaliz_test()


# main
# ========================================================================
def main():
    run_all_test()
    print_ko_lst()
    diff_utils.save_diff_html()


if __name__ == "__main__":
    main()
