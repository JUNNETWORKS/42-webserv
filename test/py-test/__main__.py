from . import test_case
from . import diff_utils
from . import run_test


def main():
    is_all_test_ok = test_case.run_all_test()
    run_test.all_test_result()
    diff_utils.save_diff_html()
    exit(is_all_test_ok == False)


if __name__ == "__main__":
    main()
