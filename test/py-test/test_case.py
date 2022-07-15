from . import response_class as res
from . import cmd_args
from .run_test import run_test
from .run_test import run_cmp_test
from .run_test import is_test_success

# TEST CASE
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
    expect_response = res.response(404, file_path="public/error_pages/404.html")
    run_test("/NotExist", expect_response)
    run_test("/NotExist/NotExist", expect_response)
    run_test("/hoge/NotExist", expect_response)


def index_test():
    expect_response = res.response(200, file_path="public/index-test-dir/index.html")
    run_test("/index-test-dir/", expect_response=expect_response)
    run_test("/index-test-dir/index.html", expect_response=expect_response)
    expect_response = res.response(
        200, file_path="public/index-test-dir/not_index.html"
    )
    run_test("/index-test-dir/not_index.html", expect_response=expect_response)


def autoindex_test():
    req_path = "/"
    expect_response = res.response(200)
    run_test(req_path, expect_response, ck_body=False)


def path_normaliz_test():
    # 200 /
    expect_response = res.response(200)
    run_test("/", expect_response=expect_response, ck_body=False)
    run_test("/.", expect_response=expect_response, ck_body=False)
    run_test("/.", expect_response=expect_response, ck_body=False)
    run_test("/./", expect_response=expect_response, ck_body=False)
    run_test("/sample.html/..", expect_response=expect_response, ck_body=False)
    run_test("/sample.html/../", expect_response=expect_response, ck_body=False)

    # 200 /sample.html
    expect_response = res.response(200, file_path="public/sample.html")
    run_test("/sample.html", expect_response)
    run_test("///sample.html", expect_response)
    run_test("/./././sample.html", expect_response)
    run_test("/NotExist/../sample.html", expect_response)

    # 404
    expect_response = res.response(404)
    run_test("/sample.html/.", expect_response=expect_response, ck_body=False)
    run_test("/sample.html/NotExist/.", expect_response=expect_response, ck_body=False)
    run_test("/sample.html/NotExist/..", expect_response=expect_response, ck_body=False)
    run_test(
        "/sample.html/NotExist/../", expect_response=expect_response, ck_body=False
    )

    # bad req
    expect_response = res.response(400)
    run_test("/..", expect_response=expect_response, ck_body=False)
    run_test("/../", expect_response=expect_response, ck_body=False)
    run_test("/NotExist/../..", expect_response=expect_response, ck_body=False)
    run_test("/NotExist/../../..", expect_response=expect_response, ck_body=False)
    run_test("///../../..", expect_response=expect_response, ck_body=False)

    # long req
    expect_response = res.response(414)
    s = "/" * 10000
    run_test(s, expect_response=expect_response, ck_body=False)
    s = "/" + "a" * 10000
    run_test(s, expect_response=expect_response, ck_body=False)





# TODO : content_type を 見るようにする。
def content_type_test():
    expect_response = res.response(
        200, file_path="public/extension/file.not_exist_extension"
    )
    run_test("/extension/file.not_exist_extension", expect_response=expect_response)

    expect_response = res.response(200, file_path="public/extension/file.")
    run_test("/extension/file.", expect_response=expect_response)
    expect_response = res.response(200, file_path="public/extension/file..")
    run_test("/extension/file..", expect_response=expect_response)


def cmp_test():
    run_cmp_test("/sample.html", expect_port=cmd_args.NGINX_PORT, save_diff=True)


# Exec Test
# ========================================================================
def exec_test(f, must_all_test_ok=True):
    test_func_name = f.__name__
    print("---", test_func_name.upper(), "---")
    f()
    print()
    is_test_ok = is_test_success(test_func_name)
    if must_all_test_ok == False:
        return True
    return is_test_ok


def run_all_test() -> bool:
    is_all_test_ok = True
    is_all_test_ok &= exec_test(simple_test)
    is_all_test_ok &= exec_test(not_found_test)
    is_all_test_ok &= exec_test(index_test)
    is_all_test_ok &= exec_test(autoindex_test)
    is_all_test_ok &= exec_test(path_normaliz_test)
    is_all_test_ok &= exec_test(content_type_test, must_all_test_ok=False)
    is_all_test_ok &= exec_test(cmp_test, must_all_test_ok=False)
    return is_all_test_ok
