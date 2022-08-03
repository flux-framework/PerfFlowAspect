#!/usr/bin/env python

import os.path
import time
from perfflowaspect import aspect

from concurrent.futures import ThreadPoolExecutor
from time import sleep

pool = ThreadPoolExecutor(3)


def bar(message):
    aspect.async_event("before", "bar", filename)
    time.sleep(2)
    aspect.async_event("after", "bar", filename)
    return message


def foo():
    aspect.sync_event("before", "foo", filename)
    time.sleep(1)
    future = pool.submit(bar, ("hello"))
    while not future.done():
        time.sleep(1)
    print(future.done())
    print(future.result())
    aspect.sync_event("after", "foo", filename)


def main():
    foo()


if __name__ == "__main__":
    filename = os.path.basename(__file__)
    main()
