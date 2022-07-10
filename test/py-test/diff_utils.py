import difflib
import sys

MAX_DIFF_HTML_SIZE = 10 * 1000 * 1000

diff_html = ""


def make_diff_html(s1, s2):
    global diff_html
    df = difflib.HtmlDiff()
    diff_html += df.make_file(s1.split("\n"), s2.split("\n"))
    if len(diff_html) > MAX_DIFF_HTML_SIZE:
        print("diff_html too long", file=sys.stderr)
        exit(1)


def get_diff_ratio(s1, s2):
    s = difflib.SequenceMatcher(None, s1, s2)
    return round(s.ratio(), 2)


def save_diff_html():
    with open("diff.html", "w") as file:
        file.write(diff_html)
