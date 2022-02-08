*NOTE: The interfaces are being actively developed and
are not yet stable.* The github issue tracker is the primary way to
communicate with the developers.

[![Build Status](https://github.com/flux-framework/PerfFlowAspect/actions/workflows/github-actions.yml/badge.svg)](https://github.com/flux-framework/PerfFlowAspect/actions)
[![Code Style: Black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

## PerfFlowAspect: a tool to analyze cross-cutting performance concerns of composite scientific workflows.


### Overview
High performance computing (HPC) researchers are increasingly
introducing and composing disparate workflow-management
technologies and components to create scalable end-to-end
science workflows.
These technologies have generally been developed in isolation and often
feature widely varying levels of performance, scalability
and interoperability. All things considered, optimizing
the end-to-end workflow amidst those considerations
is a highly daunting task and thus it requires
effective performance analysis techniques and tools.

Unfortunately, there still is a paucity of techniques and
tools that can analyze the end-to-end performance
of such a composite workflow.  While a myriad of analysis
tools exist for traditional HPC programming
paradigms (e.g., a single application running at scale), there
has been a lack of studies and tools to understand
the effectiveness and efficiency of this emerging workflow
paradigm.

Enter PerfFlowAspect. It is a simple Aspect-Oriented Programming-based tool
that can cast a cross-cutting performance-analysis concern
 or *aspect* across a heterogeneous set of components
(e.g,, combining Maestro and custom workflow pipeline
with Flux along with microservices running on on-premises
kubernetes machines)
used to create a modern-day composite science workflow.

PerfFlowAspect will provide multiple language support most relevant
for HPC workflows including Python. It is designed specifically
to allow researchers to weave the performance aspect into
critical points of execution across many workflow components
without having to lose the modularity and uniformity
as to how performance is measured and controlled.

### Quick Start
`perfflow` is the python package that contains
the PerfFlowAspect tool for the python language.
`src/python/perfflowaspect/aspect.py` contains a key annotating
decorator. Users can use the
`@perfflowaspect.aspect.critical_path()` decorator
to annotate their
functions that are likely to be on the critical path
of the workflow's end-to-end performance.
These annotated functions then serve as
the [Join points](https://en.wikipedia.org/wiki/Join\_point)
that can be weaved with PerfFlowAspect to be acted upon.

The following shows a simple snippet that
annotates two functions.

```python
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
```

Once annotated, running this python code will produce
a performance trace data file named
`perfflow.<hostname>.<pid>`. It uses [Chrome Tracing
Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview)
in JSON so that it can be loaded into Google
Chrome Tracing to render the critical path events
on the global tracing timeline.
Please see [here](http://www.gamasutra.com/view/news/176420/Indepth_Using_Chrometracing_to_view_your_inline_profiling_data.php) where Colt McAnlis actually
wrote about using Chrome Tracing for game profiling.


### Design and mechanics of PerfFlowAspect

When these annotated functions, or join points,
are weaved with PerfFlowAspect, we can
invoke specific performance-analysis actions,
a piece of tracing code, on those points of execution.
They are often referred to as
*advice* in Aspect Oriented Programming.

PerfFlowAspect currently supports only one type
of advice: **ChromeTracingAdvice**.
`src/python/perfflowaspect/advice_chrome.py` implements
this advice class.
This particular advice simply logs a performance event data
in CTF.

PerfFlowAspect's annotation also supports
the notion of pointcut: a predicate that matches join points.
In fact, `@perfflowaspect.aspect.critical_path()` can take an
optional keyword argument called `pointcut` whose value
can be `around`, `before`, `after` or their
async variations (`around_async`, `before_async`,
and `after_async`).
`pointcut=around` will invoke the advice before and
after the execution of the annotated function whereas
`pointcut=before` or `pointcut=after` will only advise
either corresponding point of function execution.


### License

SPDX-License-Identifier: LGPL-3.0

LLNL-CODE-764420
