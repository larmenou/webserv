import sys
import test_cases

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


if __name__ == "__main__" :
    if len(sys.argv) != 2:
        print("Usage : python tester.py <target_url>")
        exit(1)
    url = sys.argv[1]
    for func in test_cases.test_cases:
        print('\n\n'+'-' * 32)
        print(f"- Testing ", func[0])
        if (func[1](url)) == False :
            print(f"[{bcolors.FAIL}FAILED{bcolors.ENDC}]")
        else :
            print(f"[{bcolors.OKGREEN}SUCCESS]{bcolors.ENDC}")
        input(f'Press a key to continue')