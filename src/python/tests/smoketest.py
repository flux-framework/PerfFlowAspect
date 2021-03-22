#! /usr/bin/python3

import time
import perfflow
import perfflow.aspect


@perfflow.aspect.critical_path()
def bar(message):
    time.sleep(1)
    print(message)


@perfflow.aspect.critical_path()
def foo():
    time.sleep(2)
    bar("hello")


def main():
    foo()


if __name__ == "__main__":
    main()
