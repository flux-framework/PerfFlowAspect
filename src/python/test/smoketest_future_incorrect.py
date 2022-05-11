#!/usr/bin/env python3

import time
import perfflowaspect
import perfflowaspect.aspect

from concurrent.futures import ThreadPoolExecutor
from time import sleep

pool = ThreadPoolExecutor(3)


@perfflowaspect.aspect.critical_path(pointcut="around_async_typo")
def bar(message):
    sleep(3)
    return message


@perfflowaspect.aspect.critical_path()
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
