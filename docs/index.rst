..
   # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

################
 PerfFlowAspect
################

PerfFlowAspect is a tool to analyze cross-cutting performance concerns of
composite scientific workflows.

**************
 Introduction
**************

High performance computing (HPC) researchers are increasingly introducing and
composing disparate workflow-management technologies and components to create
scalable end-to-end science workflows. These technologies have generally been
developed in isolation and often feature widely varying levels of performance,
scalability and interoperability. All things considered, optimizing the
end-to-end workflow amidst those considerations is a highly daunting task and
thus it requires effective performance analysis techniques and tools.

Unfortunately, there still is a paucity of techniques and tools that can analyze
the end-to-end performance of such a composite workflow. While a myriad of
analysis tools exist for traditional HPC programming paradigms (e.g., a single
application running at scale), there has been a lack of studies and tools to
understand the effectiveness and efficiency of this emerging workflow paradigm.

Enter PerfFlowAspect. It is a simple Aspect-Oriented Programming-based (AOP)
tool that can cast a cross-cutting performance-analysis concern or aspect across
a heterogeneous set of components (e.g, combining Maestro and a custom workflow
pipeline with Flux along with microservices running on on-premises Kubernetes
machines) used to create a modern-day composite science workflow.

PerfFlowAspect will provide multi-language support, particularly for those most
relevant in HPC workflows including Python. It is designed specifically to allow
researchers to weave the performance aspect into critical points of execution
across many workflow components without having to lose the modularity and
uniformity as to how performance is measured and controlled.

**********************************
 PerfFlowAspect Project Resources
**********************************

**Online Documentation** https://perfflowaspect.readthedocs.io/

**Github Source Repo** https://github.com/flux-framework/PerfFlowAspect.git

**Issue Tracker** https://github.com/flux-framework/PerfFlowAspect/issues

**************
 Contributors
**************

-  Dong H. Ahn
-  Stephanie Brink
-  James Corbett
-  Stephen Herbein (NVIDIA)
-  Aliza Lisan (University of Oregon)
-  Francesco Di Natale (NVIDIA)
-  Tapasya Patki

******************************
 PerfFlowAspect Documentation
******************************

.. toctree::
   :maxdepth: 2
   :caption: Basics

   BasicTutorial
   Releases
   Licenses

.. toctree::
   :maxdepth: 2
   :caption: Reference

   BuildingPerfFlowAspect
   Annotations
   UpcomingFeatures

.. toctree::
   :maxdepth: 2
   :caption: Contributing

   PerfFlowAspectDevel

********************
 Indices and tables
********************

-  :ref:`genindex`
-  :ref:`modindex`
-  :ref:`search`
