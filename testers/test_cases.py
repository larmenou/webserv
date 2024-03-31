import requests
import os

test_cases = []

def unknownMethod(url : str):
    resp = requests.request("POUET", url)
    print(f"Status : {resp.status_code}")
    print(f"Response : \n{resp.text}\n")
    if resp.status_code != 501 :
        return False
    return True

def putNormal(url : str):
    data = "This should be in the file just created."
    resp = requests.put(url + "/uploads/new_file", data=data)
    print(f"Status : {resp.status_code}")
    res = open("../tmp/new_file").read()
    os.system("rm -f ../tmp/new_file")
    if resp.status_code != 201 and res != data:
        return False
    return True

def putForbidden(url : str):
    os.system("touch ../tmp/forbidden")
    os.system("chmod 000 ../tmp/forbidden")
    resp = requests.put(url + "/uploads/forbidden", data="blablabla")
    print(f"Status : {resp.status_code}")
    os.system("rm -f ../tmp/forbidden")
    if resp.status_code != 403 :
        return False
    return True

def putDir(url : str):
    os.system("mkdir ../tmp/dir")
    resp = requests.put(url + "/uploads/dir", data="blablabla")
    print(f"Status : {resp.status_code}")
    os.system("rm -rf ../tmp/dir")
    if resp.status_code != 403 :
        return False
    return True

def getNormal(url : str):
    resp = requests.get(url + "/index.html")
    print(f"Status : {resp.status_code}")
    print(f"Response : \n{resp.text}\n")
    if resp.status_code != 200 :
        return False
    return True

def getForbidden(url : str):
    os.system("touch ../tmp/forbidden")
    os.system("chmod 000 ../tmp/forbidden")
    resp = requests.get(url + "/uploads/forbidden")
    print(f"Status : {resp.status_code}")
    os.system("rm -f ../tmp/forbidden")
    if resp.status_code != 403 :
        return False
    return True

def getNothing(url : str):
    resp = requests.get(url + "/uploads/oisajoigjsa")
    print(f"Status : {resp.status_code}")
    print(f"Response {resp.text[:400]}")
    if resp.status_code != 404 :
        return False
    return True


def postNormal(url : str):
    resp = requests.post(url, data="a"*100_000)
    print(f"Status : {resp.status_code}")
    print(f"Response {resp.text[:400]}")
    if resp.status_code != 200 :
        return False
    return True

def redirection(url : str):
    resp = requests.get(url + "/redirect", allow_redirects=False)
    print(f"Status : {resp.status_code}")
    if resp.status_code != 301 :
        return False
    return True


def cgi(url : str):
    resp = requests.post(url + "/cgi/test.php?test=this_is_a_test", data="hello=hiiiiii")
    print(f"Status : {resp.status_code}")
    print(f"Response {resp.text[:400]}")
    if resp.status_code != 200 or resp.text.find("<?php") != -1:
        return False
    return True

def dirTraversal(url : str):
    resp = requests.get(url + "/../../../../../../etc/passwd")
    print(f"Status : {resp.status_code}")
    if resp.status_code != 404 :
        return False
    return True

test_cases.append(("Unknown method", unknownMethod))
test_cases.append(("PUT", putNormal))
test_cases.append(("forbidden PUT", putForbidden))
test_cases.append(("PUT directory",putDir))
test_cases.append(("GET", getNormal))
test_cases.append(("Forbidden GET", getForbidden))
test_cases.append(("GET 404", getNothing))
test_cases.append(("POST", postNormal))
test_cases.append(("Redirection", redirection))
test_cases.append(("Directory traversal", dirTraversal))
test_cases.append(("CGI GET/POST", cgi))

