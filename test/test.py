import socket
import io
import time
import sys
import difflib
import argparse

# import requests
import urllib.request

WEBSERV_PORT = 49200
NGINX_PORT = 49201
APACHE_PORT = 49202

parser = argparse.ArgumentParser()
parser.add_argument("--FT__PORT", type=int, default=WEBSERV_PORT)
parser.add_argument("--ORI_PORT", type=int, default=NGINX_PORT)
parser.add_argument("--ORI_CGI_PORT", type=int, default=APACHE_PORT)
parser.add_argument("--TIMEOUT", type=int, default=10)

args = parser.parse_args()

FT__PORT = args.FT__PORT
ORI_PORT = args.ORI_PORT
ORI_CGI_PORT = args.ORI_CGI_PORT
TIMEOUT = args.TIMEOUT

GREEN = "\033[32m"
RED = "\033[31m"
RESET = "\033[39m"

OK_MSG = GREEN + "OK" + RESET
KO_MSG = RED + "KO" + RESET

TIMEOUT_MSG = "\nTIMEOUT\n"

#
def send_req(req_path, port):
    if req_path[0] != "/":
        print("send_req invalid req_path", req_path)
        return -1, "invalid req_path"

    url = "http://127.0.0.1:" + str(port) + req_path
    req = urllib.request.Request(url)

    try:
        with urllib.request.urlopen(req, timeout=TIMEOUT) as res:
            code = res.code
            body = res.read().decode()
    except urllib.error.HTTPError as err:
        print(err)
        code = err.code
        body = err.read().decode()
    except urllib.error.URLError as err:
        print(err)
        code = -1
        body = "\nerror.URLError\n"
    except socket.timeout:
        code = -1
        body = TIMEOUT_MSG
    return code, body


# socket 通信でデータ送信
# req.txt などから読み込んで request を送信
def send_socket_request(send_data, port):
    timeout = TIMEOUT
    try:
        s = socket.socket(socket.AF_INET)
        s.settimeout(timeout)
        s.connect(("localhost", port))
        s.send(send_data.encode("utf-8"))
        time.sleep(0.1)
        return s.recv(10000).decode("utf-8")
    except socket.timeout:
        print(TIMEOUT_MSG)
        return TIMEOUT_MSG


def get_status_line(res):
    status_line = ""
    splited = res.split("\n")
    for line in splited:
        if "HTTP" in line:
            status_line += line + "\n"
    return status_line


def get_file_data(file_path):
    file_data = ""
    with io.open(file_path, "r", newline="") as f:
        file_data = f.read()
    return file_data


def separate_head_body(s):
    pos = s.find("\r\n\r\n")
    if pos == -1:
        return s, ""
    else:
        return s[:pos], s[pos + 4 :]


def check_body(ft__res_body, ori_res_body):
    if ft__res_body == ori_res_body:
        return True
    else:
        return False


# ステータスラインのみチェック
def check_status_line(ft__res_head, ori_res_head):
    if get_status_line(ft__res_head) == get_status_line(ori_res_head):
        return True
    else:
        return False


# 複数のレスポンスの時に対応する必要あり
def lst_replace(s, replace_lst):
    for set in replace_lst:
        s = s.replace(set[0], set[1])
    return s


# difflib utils
# ========================================================================

diff_html = ""


def make_diff_html(s1, s2):
    global diff_html
    df = difflib.HtmlDiff()
    diff_html += df.make_file(s1.split("\n"), s2.split("\n"))
    if len(diff_html) > 10 * 1000 * 1000:
        print(len(diff_html))
        print("diff_html too long")
        exit(1)


def get_diff_ratio(s1, s2):
    s = difflib.SequenceMatcher(None, s1, s2)
    return round(s.ratio(), 2)


def save_diff_html():
    with open("diff.html", "w") as file:
        file.write(diff_html)


# run_test_for_file
# ========================================================================
# 修正中...
# # ファイルの読み込みおよびreplace
# def run_test_for_file(test_file_path, replace_lst=[]):
#     send_data = get_file_data(test_file_path)
#     send_data = lst_replace(send_data, replace_lst)

#     # webserv と nginxに request を送信
#     try:
#         ft__res = send_socket_request(send_data, FT__PORT)
#         ori_res = send_socket_request(send_data, ORI_PORT)
#     except ConnectionRefusedError:
#         print("----- ConnectionRefusedError -----")
#         print("TEST FILE    :", test_file_path, replace_lst)
#         print("\n", send_data, "\n")
#         exit(1)

#     # 複数レスポンスに対応する必要あり
#     # レスポンスをheadとbodyに分割
#     ft__res_head, ft__res_body = separate_head_body(ft__res)
#     ori_res_head, ori_res_body = separate_head_body(ori_res)

#     # headとbodyの結果確認
#     is_status_line_ok = check_status_line(ft__res_head, ori_res_head)
#     is_message_body_ok = check_body(ft__res_body, ori_res_body)

#     print("TEST FILE    :", test_file_path, replace_lst)
#     print("STATUS LINE  :", is_status_line_ok)
#     print("MESSAGE BODY :", is_message_body_ok)
#     print()

#     if (is_status_line_ok and is_message_body_ok) == False:
#         make_diff_html("", send_data)
#         make_diff_html(ft__res, ori_res)

#     return is_status_line_ok and is_message_body_ok


def run_req_test(
    req_path, ft__port=FT__PORT, ori_port=ORI_PORT, ck_code=True, body_ratio=1.0
) -> bool:
    print(f"TESTING [ {req_path} ]")

    ft__res = send_req(req_path, ft__port)
    ori_res = send_req(req_path, ori_port)

    is_success = True
    is_code_ok = ft__res[0] == ori_res[0]
    is_body_ok = get_diff_ratio(ft__res[1], ori_res[1]) >= body_ratio
    if ck_code:
        is_success = is_success and is_code_ok
    if body_ratio != 0:
        is_success = is_success and is_body_ok

    log_msg = f"[ {req_path:<30} ], CODE:{is_code_ok}, BODY:{is_body_ok}, ratio {get_diff_ratio(ft__res[1], ori_res[1])}, port {ft__port}, {ori_port}"
    if is_success:
        print(f"{OK_MSG} :", log_msg)
    else:
        print(f"{KO_MSG} :", log_msg)
    make_diff_html("", "\n" + log_msg + "\n")
    make_diff_html(ft__res[1], ori_res[1])
    print()
    return is_success


# TEST_LAUNCHER
# ========================================================================
def simple_test():
    print("\n--- SIMPLE_TEST ---\n")
    run_req_test("/sample.html")
    run_req_test("/hoge/hoge.html")
    run_req_test("/NotExist", body_ratio=0)  # code だけあってればいいので、 ratio 0


def path_normaliz_test():
    print("\n--- PATH_NORMALIZ_TEST ---\n")
    run_req_test("///")
    run_req_test("./")
    run_req_test("/././.")
    run_req_test("/..")
    run_req_test("/../")
    run_req_test("/NotExist/../..")
    run_req_test("/NotExist/../sample.html")
    run_req_test("/NotExist/NotExist/../../sample.html")


def autoindex_test():
    print("\n--- AUTOINDEX_TEST ---\n")
    # TODO : ratio 1 にする予定。
    run_req_test("/", body_ratio=0.99)
    run_req_test("/hoge/", body_ratio=0.95)


# python HTTPServer だとローカルリダイレクトとかできないらしい。
def cgi_test():
    print("\n--- CGI_TEST ---\n")
    run_req_test("/cgi-bin/simple-cgi", ori_port=ORI_CGI_PORT)
    # run_req_test("/cgi-bin/test-cgi", ori_port=ORI_CGI_PORT)

    # TODO : リダイレクト先のデータ取得してしまうので一旦保留中。
    # 後で読む
    # [Pythonのurllib.requestでリダイレクトをさせない - 日記](https://azechi-n.hatenadiary.com/entry/2022/06/21/090414)
    # run_req_test("/cgi-bin/client-redirect", ori_port=ORI_CGI_PORT)
    # run_req_test("/cgi-bin/client-redirect-with-document", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/document-response", ori_port=ORI_CGI_PORT)
    # run_req_test("/cgi-bin/document-response-with-status", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/local-redirect", ori_port=ORI_CGI_PORT)


# '/' %2f
# '.' %2e
# 'A' %41
def cgi_parse_test():
    print("\n--- CGI_PARSE_TEST ---\n")
    # シンプルケース
    run_req_test("/cgi-bin/test-cgi", ori_port=ORI_CGI_PORT)

    # ? より後ろが QUERY_STRING になる。
    run_req_test("/cgi-bin/test-cgi?hoge", ori_port=ORI_CGI_PORT)

    # + でコマンドライン引数が区切られる。
    run_req_test("/cgi-bin/test-cgi?arg1+arg2+arg3", ori_port=ORI_CGI_PORT)

    # = が ある時はコマンドライン引数なしになる。
    # QUERY_STRINGにはセットされる。
    run_req_test("/cgi-bin/test-cgi?arg1+arg2+arg3+hoge=fuga", ori_port=ORI_CGI_PORT)

    # コマンドライン引数にはデコードされた、文字列が入り、
    # QUERY_STRING には、デコード前の文字列が入る。
    run_req_test("/cgi-bin/test-cgi?A%20C", ori_port=ORI_CGI_PORT)

    # QUERY_STRING にはパーセントデコードで、エラーが起きようがセットされる。
    # TODO : 現状はargvでのデコードで、エラーを返している。
    run_req_test("/cgi-bin/test-cgi/?%", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/test-cgi/?%0", ori_port=ORI_CGI_PORT)

    # %00
    run_req_test("/cgi-bin/test-cgi/?%00%", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/test-cgi/?A%01%C", ori_port=ORI_CGI_PORT)

    # path_info
    run_req_test("/cgi-bin/test-cgi/hoge", ori_port=ORI_CGI_PORT)

    # path_info に スラッシュが入る。
    run_req_test("/cgi-bin/test-cgi/", ori_port=ORI_CGI_PORT)

    # path_info には デコードされた文字が入る。
    run_req_test("/cgi-bin/test-cgi/%41%42%43", ori_port=ORI_CGI_PORT)

    # path_info デコードで無効な文字が含まれていた場合、エラー
    # TODO :
    run_req_test("/cgi-bin/test-cgi/%", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/test-cgi/%00", ori_port=ORI_CGI_PORT)
    run_req_test("/cgi-bin/test-cgi/%2f", ori_port=ORI_CGI_PORT)


# main
# ========================================================================
def run_all_test():
    simple_test()
    path_normaliz_test()
    autoindex_test()
    cgi_test()
    cgi_parse_test()


def main():
    run_all_test()
    save_diff_html()


if __name__ == "__main__":
    main()
