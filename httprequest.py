import ctypes
from time import strptime, struct_time

class Response:
    def __init__(self, status_code: int, status_code_name: str, content_type: str, last_modified: struct_time, data: str):
        self.status_code = status_code
        self.__status_code_name = status_code_name
        self.content_type = content_type
        self.last_modified = last_modified
        self.data = data

    def __str__(self): return f"<{self.status_code} {self.__status_code_name}>"

    def __repr__(self): return f"<{self.status_code} {self.__status_code_name}>"


def __cfunction(f, argtypes: list[ctypes._SimpleCData], restype: ctypes._SimpleCData):
    res = f
    res.argtypes = argtypes
    res.restype = restype
    return res

def __prop_find(props: list[str], name: str):
    i = 0
    while props[i].split(":")[0] != name:
        i += 1
        if i >= len(props): return None

    return props[i].split(": ")[1]

lib = ctypes.CDLL("./bin/main.so")

__get = __cfunction(lib.send_get_request, [ctypes.c_char_p, ctypes.c_char_p], ctypes.c_char_p)

def get(url: str):
    if url.__contains__("https://"): url = url[8:]
    host = url.split("/")[0]
    page = "/"
    for s in url.split("/")[1:]: page += s + "/"
    # if len(page) > 1: page = page[:-1]

    response_data: str = __get(host.encode(), page.encode()).decode("latin-1")
    lines = response_data.split("\r\n")
    response_info_lines: list[str] = []

    i = 0
    while lines[i] != "":
        response_info_lines.append(lines[i])
        i += 1

    res = Response(
        int(response_info_lines[0].split(" ")[1]),
        response_info_lines[0].split(" ")[2],
        __prop_find(response_info_lines, "Content-Type").split(";")[0]
        if __prop_find(response_info_lines, "Content-Type") else None,
        strptime(
            __prop_find(response_info_lines, "Last-Modified"),
            "%a, %d %b %Y %H:%M:%S GMT"
        ) if __prop_find(response_info_lines, "Last-Modified") else None,
        lines[i+1]
    )

    return res