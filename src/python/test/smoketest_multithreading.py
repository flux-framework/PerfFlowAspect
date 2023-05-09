#!/usr/bin/env python

import time
import threading

import perfflowaspect
import perfflowaspect.aspect


@perfflowaspect.aspect.critical_path()
def bar(message):
    time.sleep(1)
    print(message)


@perfflowaspect.aspect.critical_path()
def foo(message):
    time.sleep(1)
    bar(message)


@perfflowaspect.aspect.critical_path()
def thd_fn(name):
    foo("Hello from Thread ID: " + str(name))


def main():
    threads = []
    for i in range(5):
        t = threading.Thread(target=thd_fn, args=(i,))
        t.start()
        threads.append(t)

    for t in threads:
        t.join()


if __name__ == "__main__":
    main()
