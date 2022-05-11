#!/usr/bin/env python3

import time
import os.path
from perfflowaspect import aspect


def foo():
    aspect.sync_event("before", "foo", filename)
    time.sleep(2)
    print("hello")
    aspect.sync_event("after", "foo", filename)


def main():
    aspect.sync_event("before", "main", filename)
    foo()
    aspect.sync_event("after", "main", filename)


if __name__ == "__main__":
    filename = os.path.basename(__file__)
    main()
