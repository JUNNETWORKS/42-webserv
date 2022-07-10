from . import test_case
from . import diff_utils
from . import run_test

# main
# ========================================================================
test_exit_code = 0


def main():
    test_case.run_all_test()
    run_test.all_test_result()
    diff_utils.save_diff_html()
    exit(test_exit_code)


if __name__ == "__main__":
    main()
