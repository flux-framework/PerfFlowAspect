.. # Copyright 2021-2022 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

#######################
Source Code Annotations
#######################

Users can annotate their workflow code to get end-to-end performance insights.
Currently, three techniques are available for this:

- Critical path annotation
- Synchronous events annotation
- Asynchronous events annotation

For critical path annotation, the user can provide pointcut and scope
information for the annotated region. Currently, valid pointcut values are
``before``, ``after``, ``around``, ``before_async``, ``after_async``, and
``around_async``.  When no pointcut is specified, the default assumption is
``around``. We show an example of this below:

.. code-block:: python

    #!/usr/bin/python3

    import time
    import perfflowaspect
    import perfflowaspect.aspect

    @perfflowaspect.aspect.critical_path()
    def foo(msg):
        print("foo")
        time.sleep(1)
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

For synchronous event annotation, the user can provide a pointcut, name,
and category for the annotated region. Valid pointcut values are ``before``,
``after``, and ``around``. The name represents a way to identify the current
function being annotated, and the category can be a filename. An example of
this is shown below:

.. code-block:: python

    #!/usr/bin/python3

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

For asynchronous event annotation, the user can provide a pointcut, name,
category, and scope for the annotated region. An example of this is shown below
with the help of futures and thread pools:

.. code-block:: python

    #!/usr/bin/python3

    import os.path
    import time
    import logging
    import threading
    from perfflowaspect import aspect

    from concurrent.futures import ThreadPoolExecutor
    from time import sleep

    pool = ThreadPoolExecutor(3)

    def bar(message):
        aspect.async_event("before", "bar", filename)
        sleep(3)
        aspect.async_event("after", "bar", filename)
        return message

    def foo():
        aspect.sync_event("before", "foo", filename)
        time.sleep(2)
        future = pool.submit(bar, ("hello"))
        while not future.done():
            sleep(1)
        print(future.done())
        print(future.result())
        aspect.sync_event("after", "foo", filename)

    def main():
        foo()

    if __name__ == "__main__":
        filename = os.path.basename(__file__)
        main()
