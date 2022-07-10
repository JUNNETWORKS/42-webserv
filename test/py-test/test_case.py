from . import response_class as res
from .run_test import run_test
from .run_test import is_test_success

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
    is_test_ok = is_test_success(test_func_name)
    if is_test_ok == False and must_all_test_ok:
        global test_exit_code
        test_exit_code = 1
    print()


def run_all_test():
    exec_test(simple_test)
    exec_test(not_found_test)
    exec_test(autoindex_test)
    exec_test(path_normaliz_test, must_all_test_ok=False)
