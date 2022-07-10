import io


def get_file_data(file_path):
    file_data = ""
    with io.open(file_path, "r", newline="") as f:
        file_data = f.read()
    return file_data
