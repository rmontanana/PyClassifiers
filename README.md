# Pyclassifiers

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](<https://opensource.org/licenses/MIT>)
![Gitea Last Commit](https://img.shields.io/gitea/last-commit/rmontanana/pyclassifiers?gitea_url=https://gitea.rmontanana.es&logo=gitea)

Python Classifiers C++ Wrapper

## 0. Setup

Before compiling PyClassifiers.

### Miniconda

To be able to run Python Classifiers such as STree, ODTE, SVC, etc. it is needed to install Miniconda. To do so, download the installer from [Miniconda](https://docs.conda.io/en/latest/miniconda.html) and run it. It is recommended to install it in the home folder.

In Linux sometimes the library libstdc++ is mistaken from the miniconda installation and produces the next message when running the b_xxxx executables:

```bash
libstdc++.so.6: version `GLIBCXX_3.4.32' not found (required by b_xxxx)
```

The solution is to erase the libstdc++ library from the miniconda installation:

### boost library

[Getting Started](<https://www.boost.org/doc/libs/1_83_0/more/getting_started/index.html>)

The best option is install the packages that the Linux distribution have in its repository. If this is the case:

```bash
sudo dnf install boost-devel
```

If this is not possible and the compressed packaged is installed, the following environment variable has to be set pointing to the folder where it was unzipped to:

```bash
export BOOST_ROOT=/path/to/library/
```

In some cases, it is needed to build the library, to do so:

```bash
cd /path/to/library
mkdir own
./bootstrap.sh --prefix=/path/to/library/own
./b2 install
export BOOST_ROOT=/path/to/library/own/
```

Don't forget to add the export BOOST_ROOT statement to .bashrc or wherever it is meant to be.

## Installation

### Prerequisites

Install Conan package manager:

```bash
pip install conan
```

### Build and Install

```bash
make release
make buildr
sudo make install
```
