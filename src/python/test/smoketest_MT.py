#! /usr/bin/python3

import time
import threading
import perfflowaspect
import perfflowaspect.aspect


@perfflowaspect.aspect.critical_path()
def bar(message):
    time.sleep(1)


@perfflowaspect.aspect.critical_path()
def foo(message):
    time.sleep(2)
    bar(message)


def thread_function(name):
    foo("hello")


def main():
    threads = []
    for i in range(10):
        t = threading.Thread(target=thread_function, args=(i,))
        t.start()
        threads.append(t)

    for t in threads:
        t.join()


if __name__ == "__main__":
    main()
