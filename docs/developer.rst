############################
 PerfFlowAspect Development
############################

This is a short developer guide for using the included development environment.
While we do not provide VSCode (there was `a bug we could not figure out
<https://github.com/flux-framework/PerfFlowAspect/issues/112>`_) we provide a
Dockerfile that makes it easy to build and use PerfFlow!

********************
 Building Container
********************

You can build the container from scratch:

.. code:: console

   $ docker build -t ghcr.io/flux-framework/perf-flow-aspect .

Note that for a faster build (or no build!), you can pull or use an existing
container:

.. code:: console

   $ docker pull ghcr.io/flux-framework/perf-flow-aspect

To shell into the container:

.. code:: console

   $ docker run -it ghcr.io/flux-framework/perf-flow-aspect

Once inside, you can cd to where the tests are, and run a few!

.. code:: console

   cd src/c/build/test
   ./t0001-cbinding-basic.t

Note that if you don't have a GPU, these probably will error (I do not)! Here is
how to run Python tests:

.. code:: console

   cd src/python/test
   ./t0001-pybinding-basic.t

You'll again have issues without an actual GPU. And that's it! Note that we will
update this documentation as we create more examples.
