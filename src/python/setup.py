##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

from os import path
from setuptools import setup


def load_readme():
    """
    Load the readme from the root of the package directory.

    :returns: A string containing the contents of README.md.
    """
    pkg_path = path.abspath(path.dirname(__file__))
    with open(path.join(pkg_path, "README.md")) as f:
        long_description = f.read()

    return long_description


setup(
    name="perfflowaspect",
    description="tool to analyze cross-cutting performance concerns of "
    "composite scientific workflows.",
    version="0.0.1",
    author="Dong H. Ahn, Stephen Herbein, James Corbett, Francesco Di Natale",
    author_email="ahn1@llnl.gov, herbein1@llnl.gov, corbett8@llnl.gov, dinatale3@llnl.gov",
    package_dir={
        '': 'src/python'
    },
    packages=['perfflowaspect'],
    entry_points={},
    install_requires=[],
    extras_require={},
    long_description=load_readme(),
    long_description_content_type="text/markdown",
    python_requires=">=3.6",
    classifiers=[
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering",
        "Topic :: System :: Distributed Computing",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
    ],
)
