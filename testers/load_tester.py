import requests
import threading
import sys

NB_CLIENTS  = 200
NB_REQ      = 10

def put(target : str) -> None:
    requests.put(target, data="a" * 10)

def post(target : str) -> None:
    requests.post(target, data="a" * 10)

def get(target : str) -> None:
    requests.get(target)

def delete(target : str) -> None:
    requests.delete(target)

def client(target : str, func, id : int) -> None:
    for i in range(0, NB_REQ):
        try :
            func(target)
        except:
            fails[id] += 1

METHOD = {
    "PUT" : put,
    "POST" : post,
    "GET" : get,
    "DELETE" : delete,
}

def load_test(target : str, method) -> None:
    print(f"Starting tests for {method} ...")
    if not (method in METHOD):
        print("Invalid method")
        exit(1)
    func = METHOD[method]
    for i in range(NB_CLIENTS):
        threads.append(threading.Thread(target=client, args=[target, func, i]))
    for i in threads:
        i.start()
    for i in threads:
        i.join()
    print(f'Success rate {100 - (sum(fails) / (NB_CLIENTS * NB_REQ)) * 100}%')

if __name__ == "__main__" :
    threads = []
    fails   = [0] * NB_CLIENTS

    if len(sys.argv) != 3:
        print("Usage : python load_tester.py <target_url> <method>")
        exit(1)
    load_test(sys.argv[1], sys.argv[2])
