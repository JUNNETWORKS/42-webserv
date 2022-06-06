import socket
import io
import time
import sys

WEBSERV_PORT = 49200
NGINX_PORT = 49201
APACHE_PORT = 49202


def send_request(test_file, port):
    s = socket.socket(socket.AF_INET)
    s.connect(("localhost", port))
    with io.open(test_file, "r", newline="") as f:
        a = f.read()
        # print(a.encode("utf-8"))
        s.send(a.encode("utf-8"))
    time.sleep(0.1)

    return s.recv(10000).decode("utf-8")


def print_response_status_code(res):
    splited = res.split("\n")
    for line in splited:
        if "HTTP" in line:
            print(line)
    print()


def main():
    if len(sys.argv) == 2:
        test_file = sys.argv[1]
    else:
        test_file = "./req/req1.txt"

    print("TEST FILE :", test_file, file=sys.stderr)
    print()

    webserv_res = send_request(test_file, WEBSERV_PORT)
    nginx_res = send_request(test_file, NGINX_PORT)
    apache_res = send_request(test_file, APACHE_PORT)

    print("--- WEBSERV RES ---")
    # print(webserv_res)
    print_response_status_code(webserv_res)

    print("--- NGINX ---")
    # print(nginx_res)
    print_response_status_code(nginx_res)

    print("--- APACHE ---")
    # print(apache_res)
    print_response_status_code(apache_res)


if __name__ == "__main__":
    main()
