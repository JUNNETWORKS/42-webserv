from . import io_utils

# response
# ========================================================================
class response:
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


def is_eq_response(response1, response2, ck_code=True, ck_body=True) -> bool:
    is_code_ok = True
    is_body_ok = True
    if ck_code:
        is_code_ok = response1.code == response2.code
    if ck_body:
        is_body_ok = response1.body == response2.body
    return is_code_ok and is_body_ok
