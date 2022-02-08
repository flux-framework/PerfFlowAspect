.. # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

#################
Quick Start Guide
#################

PerfFlowAspect is based on Aspect-Oriented Programming (AOP). Some common terms
used in AOP are described below.

- **Join Points**: A point in a program's execution in which the behavior can
  be modified by AOP. The candidate points are method invocations.
- **Pointcuts**: An expression used to match join points (creating a set of
  join points), allows you to specify to AOP where and when in the code to make
  modifications.
- **Advice**: Defines the additional behavior or action that will be inserted into
  the code, specifically at each join point matched by the pointcut.
- **Aspect**: The collection of the pointcut expression and the advice.
- **Weaver**: Applies aspects into the code, modifying code at join points with
  matching pointcuts and associated advices. The combining of aspects and code
  enables execution of cross-cutting concerns.

PerfFlowAspect relies on annotated functions in the user's source code and can
invoke specific performance-analysis actions, a piece of tracing code, etc. on
those points of execution. In AOP, these trigger points are called join points
in the source code, and the functionality invoked is called advice.

The python package ``perfflowaspect`` contains the PerfFlowAspect tool for the
Python language. The file ``src/python/perfflowaspect/aspect.py`` contains a
key annotating decorator. Users can use the
``@perfflowaspect.aspect.critical_path()`` decorator to annotate their
functions that are likely to be on the critical path of the workflow's
end-to-end performance. These annotated functions then serve as the join points
that can be weaved with PerfFlowAspect to be acted upon.

The following shows a simple snippet that annotates two functions.

.. code-block:: python

    import perfflowaspect.aspect

    @perfflowaspect.aspect.critical_path()
    def bar(message):
        time.sleep(1)
        print(message)

    @perfflowaspect.aspect.critical_path()
    def foo():
        time.sleep(2)
        bar("hello")

    def main():
        foo()

Once annotated, running this python code will produce a performance trace data
file named ``perfflow.<hostname>.<pid>``. It uses Chrome Tracing Format in JSON
so that it can be loaded into Google Chrome Tracing to render the critical path
events on the global tracing timeline, using the Perfetto visualization tool.
Details on these can be found at the links below:

- **Chrome Tracing Tool:** https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/
- **Perfetto Visualizer:** https://perfetto.dev/
