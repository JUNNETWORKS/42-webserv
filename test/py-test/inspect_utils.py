import inspect


def get_caller_func_name() -> str:
    # この関数も呼び出されるので 1 ではなく 2
    return inspect.stack()[2].function
