#!/usr/bin/python3

import time
import perfflowaspect
import perfflowaspect.aspect

from concurrent.futures import ThreadPoolExecutor
from time import sleep

pool = ThreadPoolExecutor(4)

# Introduce a dummy function work around a lack of support
# of PerfflowAspect prototype
@perfflowaspect.aspect.critical_path(pointcut="before_async", scope="foo")
def async_begin_in_foo():
    pass


@perfflowaspect.aspect.critical_path(pointcut="after_async", scope="foo")
def bar(message):
    sleep(1)
    return message


@perfflowaspect.aspect.critical_path()
def foo():
    futures = []
    time.sleep(2)

    for i in range(5):
        async_begin_in_foo()
        futures.append(pool.submit(bar, ("hello")))

    for future in futures:
        while not future.done():
            sleep(1)
        print(future.done())
        print(future.result())


def main():
    foo()


if __name__ == "__main__":
    main()
