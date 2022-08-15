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
    temp = 0
    temp1 = [25] * 10000
    temp2 = [22] * 10000
    temp3 = [0] * 10000
    for i in range(10000):
        temp = temp + 1
        temp3[i] = temp1[i] * temp2[i]
        for j in range(i + 1, 10000):
            temp3[j] = temp3[j] / 3
    bar()
    if msg == "hello":
        return 1
    return 0


def main():
    print("Inside main")
    foo("hello")
    return 0


if __name__ == "__main__":
    main()
