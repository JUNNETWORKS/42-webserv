import socket
import io
import time
import sys
import difflib

WEBSERV_PORT = 49200
NGINX_PORT = 49201


def send_request(send_data, port):
    timeout = 5
    try:
        s = socket.socket(socket.AF_INET)
        s.settimeout(timeout)
        s.connect(("localhost", port))
        s.send(send_data.encode("utf-8"))
        time.sleep(0.1)
        return s.recv(10000).decode("utf-8")
    except socket.timeout:
        print("TIMEOUT", file=sys.stderr)
        return "TIMEOUT"


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


def check_body(webserv_res_body, nginx_res_body):
    if webserv_res_body == nginx_res_body:
        return True
    else:
        return False


# ステータスラインのみチェック
def check_status_line(webserv_res_head, nginx_res_head):
    if get_status_line(webserv_res_head) == get_status_line(nginx_res_head):
        return True
    else:
        return False


# 複数のレスポンスの時に対応する必要あり
def lst_replace(s, replace_lst):
    for set in replace_lst:
        s = s.replace(set[0], set[1])
    return s


def make_diff_html(s1, s2):
    df = difflib.HtmlDiff()
    diff_html = df.make_file(s1.split("\n"), s2.split("\n"))
    return diff_html


def run_test(test_file_path, replace_lst=[]):
    # ファイルの読み込みおよびreplace
    send_data = get_file_data(test_file_path)
    send_data = lst_replace(send_data, replace_lst)

    # webserv と nginxに request を送信
    try:
        webserv_res = send_request(send_data, WEBSERV_PORT)
        nginx_res = send_request(send_data, NGINX_PORT)
    except ConnectionRefusedError:
        print("----- ConnectionRefusedError -----", file=sys.stderr)
        print("TEST FILE    :", test_file_path, replace_lst, file=sys.stderr)
        print("\n", send_data, "\n", file=sys.stderr)
        exit(1)

    # 複数レスポンスに対応する必要あり
    # レスポンスをheadとbodyに分割
    webserv_res_head, webserv_res_body = separate_head_body(webserv_res)
    nginx_res_head, nginx_res_body = separate_head_body(nginx_res)

    # headとbodyの結果確認
    is_status_line_ok = check_status_line(webserv_res_head, nginx_res_head)
    is_message_body_ok = check_body(webserv_res_body, nginx_res_body)

    print("TEST FILE    :", test_file_path, replace_lst)
    print("STATUS LINE  :", is_status_line_ok)
    print("MESSAGE BODY :", is_message_body_ok)
    print()

    if (is_status_line_ok and is_message_body_ok) == False:
        global diff_html
        diff_html += make_diff_html("", send_data)
        diff_html += make_diff_html(webserv_res, nginx_res)
        if len(diff_html) > 10 * 1000 * 1000:
            print(len(diff_html))
            print("diff_html too long", file=sys.stderr)
            exit(1)

    return is_status_line_ok and is_message_body_ok


def path_test():
    test_file_path = "req/tpl-get-path-test.txt"
    run_test(test_file_path, [["{PATH}", "/"]])
    run_test(test_file_path, [["{PATH}", "/hoge"]])
    run_test(test_file_path, [["{PATH}", "/hoge/"]])
    run_test(test_file_path, [["{PATH}", "/hoge/hoge.html"]])
    run_test(test_file_path, [["{PATH}", "/hoge/fuga.html"]])
    run_test(test_file_path, [["{PATH}", "///"]])
    run_test(test_file_path, [["{PATH}", "./"]])
    run_test(test_file_path, [["{PATH}", "/././."]])
    run_test(test_file_path, [["{PATH}", "/NotExist/"]])
    run_test(test_file_path, [["{PATH}", "/NotExist/.."]])
    run_test(test_file_path, [["{PATH}", "/NotExist/NotExist/../.."]])
    run_test(test_file_path, [["{PATH}", "/NotExist/../.."]])


diff_html = ""


def main():
    if len(sys.argv) == 2:
        test_file = sys.argv[1]
        run_test(test_file)
    else:
        test_file = "req/req1.txt"
        path_test()

    # diff.htmlの作成
    with open("diff.html", "w") as file:
        file.write(diff_html)


if __name__ == "__main__":
    main()
