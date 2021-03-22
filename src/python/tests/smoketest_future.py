#!/usr/bin/python3

import time
import logging
import threading
import perfflow
import perfflow.aspect

from concurrent.futures import ThreadPoolExecutor
from time import sleep

pool = ThreadPoolExecutor(3)


@perfflow.aspect.critical_path(pointcut="around_async")
def bar(message):
    sleep(3)
    return message


@perfflow.aspect.critical_path(pointcut="around")
def foo():
    time.sleep(2)
    future = pool.submit(bar, ("hello"))
    while not future.done():
        sleep(1)
    print(future.done())
    print(future.result())


def main():
    foo()


if __name__ == "__main__":
    main()
