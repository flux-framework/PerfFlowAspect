#!/usr/bin/env python

import time
import multiprocessing
import perfflowaspect
import perfflowaspect.aspect


@perfflowaspect.aspect.critical_path()
def bar(message):
    time.sleep(1)
    print(message)


@perfflowaspect.aspect.critical_path()
def foo(message):
    time.sleep(2)
    bar(message)


@perfflowaspect.aspect.critical_path()
def prcs_fn(name):
    foo("Hello from Process ID: " + str(name))


def main():
    procs = []
    for i in range(4):
        p = multiprocessing.Process(target=prcs_fn, args=(i,))
        p.start()
        procs.append(p)

    for p in procs:
        p.join()


if __name__ == "__main__":
    main()
