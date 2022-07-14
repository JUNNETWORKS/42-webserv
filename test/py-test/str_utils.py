def to_hex_str(s):
    hex_str = ""
    for c in s:
        hex_str += f"%{ord(c):x}"
    return hex_str
