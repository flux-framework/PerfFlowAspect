#!/usr/bin/env python

import time
import perfflowaspect
import perfflowaspect.aspect


@perfflowaspect.aspect.critical_path(pointcut="around")
def bas():
    print("bas")


@perfflowaspect.aspect.critical_path(pointcut="around")
def bar():
    print("bar")
    time.sleep(0.001)
    bas()


@perfflowaspect.aspect.critical_path()
def foo(msg):
    print("foo")
    time.sleep(0.001)
    bar()
    if msg == "hello":
        return 1
    return 0


def main():
    print("Inside main")
    for i in range(4):
        foo("hello")
    return 0


if __name__ == "__main__":
    main()
