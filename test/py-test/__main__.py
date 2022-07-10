import argparse

from . import const_str
from . import send_req_utils
from . import diff_utils
from . import io_utils
from . import inspect_utils

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


# KO
# ========================================================================
test_dict = {}
test_res_lst = []


def append_test_result(test_name, is_success, data=None):
    if test_name in test_dict:
        test_dict[test_name].append([is_success, data])
    else:
        test_dict[test_name] = []
        test_dict[test_name].append([is_success, data])


def get_success_fail_count(test_name) -> bool:
    success_count = 0
    fail_count = 0
    lst = test_dict[test_name]
    for l in lst:
        success_count += l[0] == True
        fail_count += l[0] == False
    return success_count, fail_count


def test_result(test_name) -> bool:
    success_count, fail_count = get_success_fail_count(test_name)
    return fail_count == 0


def all_test_result() -> bool:
    print()
    print("----- ALL_TEST_RESULT -----")
    print()
    all_success_count = 0
    all_fail_count = 0
    for test_name in test_dict:
        success_count, fail_count = get_success_fail_count(test_name)
        print(f"{test_name.upper():<20} SUCCESS {success_count}, FAIL {fail_count}")
        all_success_count += success_count
        all_fail_count += fail_count
    print()
    print(f"TOTAL : SUCCESS {all_success_count}, FAIL {all_fail_count}")
    return all_fail_count == 0


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
        # append_ko_lst(inspect_utils.get_caller_func_name(), [log_msg])
        diff_utils.make_diff_html(ft_res_prm.body, expect_prm.body)
    append_test_result(inspect_utils.get_caller_func_name(), is_success, log_msg)
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
    expect_prm = prms(200)
    run_test(req_path, expect_prm, ck_body=False)


def path_normaliz_test():
    expect_prm = prms(200, file_path="public/sample.html")
    run_test("///sample.html", expect_prm)
    run_test("/./././sample.html", expect_prm)
    run_test("/NotExist/../sample.html", expect_prm)


def exec_test(f, must_all_test_ok=True):
    test_func_name = f.__name__
    print("---", test_func_name.upper(), "---")
    f()

    is_test_ok = test_result(test_func_name)
    if is_test_ok == False and must_all_test_ok:
        global test_exit_code
        test_exit_code = 1
    print()


def run_all_test():
    exec_test(simple_test)
    exec_test(not_found_test)
    exec_test(autoindex_test)
    exec_test(path_normaliz_test, must_all_test_ok=False)


# main
# ========================================================================
test_exit_code = 0


def main():
    run_all_test()
    all_test_result()
    diff_utils.save_diff_html()
    exit(test_exit_code)


if __name__ == "__main__":
    main()
