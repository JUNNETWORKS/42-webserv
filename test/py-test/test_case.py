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
    expect_response = res.response(404)
    run_test("/NotExist", expect_response, ck_body=False)
    run_test("/NotExist/NotExist", expect_response, ck_body=False)
    run_test("/hoge/NotExist", expect_response, ck_body=False)


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
    # TODO :
    # expect_response = res.response(400)
    # run_cmp_test("/..",port=cmd_args.NGINX_PORT,save_diff=True)
    # run_cmp_test("/../")
    # run_test("/..", expect_response=expect_response, ck_body=False)


def cmp_test():
    run_cmp_test("/sample.html", expect_port=cmd_args.NGINX_PORT, save_diff=True)


def cmp_test():
    run_cmp_test("/sample.html", expect_port=cmd_args.NGINX_PORT, save_diff=True)


def cgi_simple_test():
    run_cmp_test("/cgi-bin/simple-cgi", expect_port=cmd_args.APACHE_PORT, save_diff=True)


def cgi_query_string_test():
    req_path = "/cgi-bin/parse-test-cgi"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # ? より後ろが QUERY_STRING になる。
    req_path = "/cgi-bin/parse-test-cgi?hoge"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # + でコマンドライン引数が区切られる。
    req_path = "/cgi-bin/parse-test-cgi?arg1+arg2+arg3"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # = が ある時はコマンドライン引数なしになる。
    # QUERY_STRINGにはセットされる。
    req_path = "/cgi-bin/parse-test-cgi?arg1+arg2%00a+arg3+hogefuga"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # コマンドライン引数にはデコードされた、文字列が入り、
    # QUERY_STRING には、デコード前の文字列が入る。
    req_path = "/cgi-bin/parse-test-cgi?A%20C"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # QUERY_STRING にはパーセントデコードで、エラーが起きようがセットされる。
    # TODO : 現状はargvでのデコードで、エラーを返している。
    req_path = "/cgi-bin/parse-test-cgi?%"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    req_path = "/cgi-bin/parse-test-cgi?%0"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    req_path = "/cgi-bin/parse-test-cgi?A%00C+argv2"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)


def cgi_path_info_test():
    # req_path = "/cgi-bin/parse-test-cgi/hoge"
    # run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    req_path = "/cgi-bin/parse-test-cgi/"
    run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)

    # req_path = "/cgi-bin/parse-test-cgi/%41%42%43"
    # run_cmp_test(req_path, expect_port=cmd_args.APACHE_PORT)


#   path_info デコードで無効な文字が含まれていた場合、エラー
#     req_path = "/cgi-bin/parse-test-cgi/%"
#     expect_response = send_req(req_path, port=cmd_args.APACHE_PORT)
#     run_test(req_path, expect_response)

#     req_path = "/cgi-bin/parse-test-cgi/%00"
#     expect_response = send_req(req_path, port=cmd_args.APACHE_PORT)
#     run_test(req_path, expect_response)

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
    is_all_test_ok &= exec_test(autoindex_test)
    is_all_test_ok &= exec_test(path_normaliz_test)
    is_all_test_ok &= exec_test(cmp_test, must_all_test_ok=False)
    return is_all_test_ok
