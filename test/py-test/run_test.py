from . import response_class as res
from . import cmd_args
from . import send_req_utils
from . import inspect_utils
from . import diff_utils
from . import const_str


OK_MSG = const_str.GREEN + "[ OK ]" + const_str.RESET
KO_MSG = const_str.RED + "[ KO ]" + const_str.RESET

# Test Stat
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
    if not test_name in test_dict:
        return 0
    lst = test_dict[test_name]
    for l in lst:
        success_count += l[0] == True
    return success_count


def get_fail_count(test_name):
    fail_count = 0
    if not test_name in test_dict:
        return 0
    lst = test_dict[test_name]
    for l in lst:
        fail_count += l[0] == False
    return fail_count


def is_test_success(test_name) -> bool:
    fail_count = get_fail_count(test_name)
    return fail_count == 0


def all_test_stat() -> bool:
    print()
    print("----- ALL_TEST_STAT -----")
    print()
    all_success_count = 0
    all_fail_count = 0
    for test_name in test_dict:
        success_count = get_success_count(test_name)
        fail_count = get_fail_count(test_name)
        print(
            f"{test_name.upper():<30} {OK_MSG} {success_count:>3},   {KO_MSG} {fail_count:>3}"
        )
        all_success_count += success_count
        all_fail_count += fail_count
    print()
    print(f"TOTAL : {OK_MSG} {all_success_count:>3}, {KO_MSG} {all_fail_count:>3}")
    return all_fail_count == 0


# Run Test
# ========================================================================
def run_test(
    req_path,
    expect_res: res.response,
    body=None,
    port=cmd_args.WEBSERV_PORT,
    ck_code=True,
    ck_body=True,
    save_diff=False,
    test_name="",
) -> bool:
    ft_res_response = send_req_utils.send_req(req_path, port, body=body)

    is_success = None
    if len(req_path) >= 50:
        log_msg = f"req_path : {req_path[:50]} ..."
    else:
        log_msg = f"req_path : {req_path}"
    if res.is_eq_response(
        ft_res_response, expect_res, ck_code=ck_code, ck_body=ck_body
    ):
        is_success = True
        print(OK_MSG, log_msg)
    else:
        is_success = False
        print(KO_MSG, log_msg)

    if test_name == "":
        test_name = inspect_utils.get_caller_func_name()
    if save_diff or is_success == False:
        diff_utils.make_diff_html("", f"test_name {test_name}\n{log_msg}\nport {port}")
        diff_utils.make_diff_html(ft_res_response.body, expect_res.body)
    append_test_result(test_name, is_success, log_msg)
    return is_success


def run_cmp_test(
    req_path,
    body=None,
    port=cmd_args.WEBSERV_PORT,
    expect_port=cmd_args.NGINX_PORT,
    save_diff=False,
):
    if cmd_args.RUN_CMP_TEST == 0:
        return

    test_name = inspect_utils.get_caller_func_name()
    expect_res = send_req_utils.send_req(req_path, port=expect_port, body=body)
    run_test(
        req_path,
        expect_res,
        body=body,
        port=port,
        save_diff=save_diff,
        test_name=test_name,
    )
