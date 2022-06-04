import socket
import io
import time
import sys

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


def print_response_status_code(res):
    splited = res.split("\n")
    for line in splited:
        if "HTTP" in line:
            print(line)
    print()


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
    webserv_res_status_line = ""
    nginx_res_status_line = ""

    for line in webserv_res_head.split("\n"):
        if "HTTP" in line:
            webserv_res_status_line += line + "\n"

    for line in nginx_res_head.split("\n"):
        if "HTTP" in line:
            nginx_res_status_line += line + "\n"

    if webserv_res_status_line == nginx_res_status_line:
        return True
    else:
        return False


# 複数のレスポンスの時に対応する必要あり
def check_result(webserv_res, nginx_res):
    webserv_res_head, webserv_res_body = separate_head_body(webserv_res)
    nginx_res_head, nginx_res_body = separate_head_body(nginx_res)

    is_status_line_ok = check_status_line(webserv_res_head, nginx_res_head)
    is_message_body_ok = check_body(webserv_res_body, nginx_res_body)

    print("STATUS LINE  :", is_status_line_ok)
    print("MESSAGE BODY :", is_message_body_ok)

    return is_status_line_ok and is_message_body_ok


def lst_replace(s, replace_lst):
    for set in replace_lst:
        s = s.replace(set[0], set[1])
    return s


def run_test(test_file_path, replace_lst=[]):
    send_data = get_file_data(test_file_path)
    send_data = lst_replace(send_data, replace_lst)

    print("TEST FILE    :", test_file_path, replace_lst)
    webserv_res = send_request(send_data, WEBSERV_PORT)
    nginx_res = send_request(send_data, NGINX_PORT)

    check_result(webserv_res, nginx_res)
    print()


def path_test():
    test_file_path = "req/tpl-get-path-test.txt"
    run_test(test_file_path, [["{PATH}", "/hoge/hoge.html"]])
    run_test(test_file_path, [["{PATH}", "/hoge/fuga.html"]])


def main():
    if len(sys.argv) == 2:
        test_file = sys.argv[1]
        run_test(test_file)
    else:
        test_file = "req/req1.txt"
        path_test()


if __name__ == "__main__":
    main()
