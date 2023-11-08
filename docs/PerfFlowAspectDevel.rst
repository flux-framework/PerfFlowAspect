###################
 Developer's Guide
###################

This is a short developer guide for using the included development environment.
We provide both a base container for `VSCode
<https://code.visualstudio.com/docs/remote/containers>`_, and a production
container with PerfFlowAspect you can use for application development outside of
that. This is done via the `.devcontainer
<https://code.visualstudio.com/docs/remote/containers#_create-a-devcontainerjson-file>`_
directory. You can follow the `DevContainers tutorial
<https://code.visualstudio.com/docs/remote/containers-tutorial>`_ where you'll
basically need to:

#. Install Docker, or compatible engine
#. Install the Development Containers extension

Then you can go to the command palette (View -> Command Palette) and select
``Dev Containers: Open Workspace in Container.`` and select your cloned
PerfFlowAspect repository root. This will build a development environment. You
are free to change the base image and rebuild if you need to test on another
operating system! When your container is built, when you open ``Terminal -> New
Terminal`` you'll be in the container, which you can tell based on being the
"vscode" user. You can then proceed to the sections below to build and test
PerfFlowAspect.

**Important** the development container assumes you are on a system with uid
1000 and gid 1000. If this isn't the case, edit the ``.devcontainer/Dockerfile``
to be your user and group id. This will ensure changes written inside the
container are owned by your user. It's recommended that you commit on your
system (not inside the container) because if you need to sign your commits, the
container doesn't have access and won't be able to. If you find that you
accidentally muck up permissions and need to fix, you can run this from your
terminal outside of VSCode:

.. code:: console

   $ sudo chown -R $USER .git/
   # and then commit

***************************
 Installing PerfFlowAspect
***************************

Once inside the development environment, you can compile PerfFlowAspect:

.. code:: console

   export PATH=/usr/local/cuda-12.1/bin/:$PATH
   cd src/c
   mkdir build
   cd build
   cmake -DCMAKE_CXX_COMPILER=clang++ -DLLVM_DIR=/usr/lib/llvm-10/cmake ..
   make
   sudo make install

If you want to run tests, cd to where the tests are, and run a few!

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
