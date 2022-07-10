import argparse

from . import response_class as res
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


def get_success_count(test_name):
    success_count = 0
    lst = test_dict[test_name]
    for l in lst:
        success_count += l[0] == True
    return success_count


def get_fail_count(test_name):
    fail_count = 0
    lst = test_dict[test_name]
    for l in lst:
        fail_count += l[0] == False
    return fail_count


def test_result(test_name) -> bool:
    fail_count = get_fail_count(test_name)
    return fail_count == 0


def all_test_result() -> bool:
    print()
    print("----- ALL_TEST_RESULT -----")
    print()
    all_success_count = 0
    all_fail_count = 0
    for test_name in test_dict:
        success_count = get_success_count(test_name)
        fail_count = get_fail_count(test_name)
        print(f"{test_name.upper():<20} SUCCESS {success_count}, FAIL {fail_count}")
        all_success_count += success_count
        all_fail_count += fail_count
    print()
    print(f"TOTAL : SUCCESS {all_success_count}, FAIL {all_fail_count}")
    return all_fail_count == 0


# run Test
# ========================================================================
def run_test(
    req_path,
    expect_response: res.response,
    port=WEBSERV_PORT,
    ck_code=True,
    ck_body=True,
) -> bool:
    code, body = send_req_utils.send_req(req_path, port)
    ft_res_response = res.response(code=code, body=body)

    is_success = None
    log_msg = f"req_path : {req_path}"
    if res.is_eq_response(
        ft_res_response, expect_response, ck_code=ck_code, ck_body=ck_body
    ):
        is_success = True
        print(OK_MSG, log_msg)
    else:
        is_success = False
        print(KO_MSG, log_msg)
        # append_ko_lst(inspect_utils.get_caller_func_name(), [log_msg])
        diff_utils.make_diff_html(ft_res_response.body, expect_response.body)
    append_test_result(inspect_utils.get_caller_func_name(), is_success, log_msg)
    return is_success


# launcher Test
# ========================================================================
def simple_test():
    req_path = "/sample.html"
    file_path = "public" + req_path
    expect_response = res.response(200, file_path=file_path)
    run_test(req_path, expect_response)

    req_path = "/hoge/hoge.html"
    file_path = "public" + req_path
    expect_response = res.response(200, file_path=file_path)
    run_test(req_path, expect_response)

    req_path = "/fuga/fuga.html"
    file_path = "public" + req_path
    expect_response = res.response(200, file_path=file_path)
    run_test(req_path, expect_response)


def not_found_test():
    expect_response = res.response(404)
    run_test("/NotExist", expect_response, ck_body=False)
    run_test("/NotExist/NotExist", expect_response, ck_body=False)
    run_test("/hoge/NotExist", expect_response, ck_body=False)


def autoindex_test():
    req_path = "/"
    expect_response = res.response(200)
    run_test(req_path, expect_response, ck_body=False)


def path_normaliz_test():
    expect_response = res.response(200, file_path="public/sample.html")
    run_test("///sample.html", expect_response)
    run_test("/./././sample.html", expect_response)
    run_test("/NotExist/../sample.html", expect_response)


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
