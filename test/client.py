import socket
import io
import time
import sys

WEBSERV_PORT = 49200
NGINX_PORT = 49201


def send_request(send_data, port):
    s = socket.socket(socket.AF_INET)
    s.connect(("localhost", port))
    s.send(send_data.encode("utf-8"))
    time.sleep(0.1)

    return s.recv(10000).decode("utf-8")


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


def main():
    if len(sys.argv) == 2:
        test_file = sys.argv[1]
    else:
        test_file = "./req.txt"

    print("TEST FILE :", test_file, file=sys.stderr)
    print()

    file_data = get_file_data(test_file)

    webserv_res = send_request(file_data, WEBSERV_PORT)
    nginx_res = send_request(file_data, NGINX_PORT)

    print("--- WEBSERV RES ---")
    # print(webserv_res)
    print_response_status_code(webserv_res)

    print("--- NGINX ---")
    # print(nginx_res)
    print_response_status_code(nginx_res)


if __name__ == "__main__":
    main()
