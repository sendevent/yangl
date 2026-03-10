# yangl build

The general description of build process. 

## Overview

- [Getting Started](#toc-getting-started)
- [Prerequisites](#toc-prerequisites)
  - [Qt](#toc-qt)
    - [Version](#toc-qt-version)
    - [Modules](#toc-qt-modules)
  - [C++](#toc-cpp)
- [Building](#toc-building)
  - [QtCreator](#toc-qtcreator)
  - [Shell](#toc-shell)
    - [Build script](#toc-build-script)
    - [Manual steps](#toc-manual-steps)
- [Packaging](#toc-packaging)
  - [.deb (Debian/Ubuntu)](#toc-deb)
  - [.rpm (Fedora/RHEL/openSUSE)](#toc-rpm)
  - [AppImage](#toc-appimage)
  - [Custom Qt path](#toc-custom-qt-path)
- [For developers](#toc-for-developers)
  - [Running tests](#toc-running-tests)
  - [Code style](#toc-code-style)

<a name="toc-getting-started"></a>
## Getting Started

I assume you already have the c++ build tools and git installed and the source code is cloned to your local file system. Or you do know how to perform that in used environment :)

```
sudo apt install git build-essential # may depend on your Linux distro
mkdir ~/yangl # the actual dir name does not matter
cd ~/yangl
git clone https://github.com/sendevent/yangl.git .
```

<a name="toc-prerequisites"></a>
## Prerequisites
<a name="toc-qt"></a>
### Qt
<a name="toc-qt-version"></a>
#### Version
6.8+

<a name="toc-qt-modules"></a>
#### Modules

* base (Core, Gui, Network, Concurrent)
* qml (Qml, Quick, QuickWidgets)
* geo (Location, Positioning)

Here's how the install command may look on Debian:


```
sudo apt install \
                qt6-base-dev \
                qt6-base-dev-tools \
                qt6-base-private-dev \
                qt6-declarative-dev \
                qt6-declarative-dev-tools \
                qt6-declarative-private-dev \
                qt6-tools-dev \
                qt6-tools-dev-tools \
                qt6-tools-private-dev \
                qt6-location-dev \
                qt6-positioning-dev \
                qt6-positioning-private-dev
```

Please refer to your distro manual to get the related packages.

<a name="toc-cpp"></a>
### C++

The most-modern-as-for-today (c++23-capable) compiller — gcc 14.2.0 or clang 19.1.7 would be enough.

<a name="toc-building"></a>
## Building

<a name="toc-qtcreator"></a>
### QtCreator

Open [CMakeLists.txt](CMakeLists.txt) and build it as a regular project.

<a name="toc-shell"></a>
### Shell

<a name="toc-build-script"></a>
#### Build script

[build.sh](build.sh) performs the build process in the sub directory `./scriptbuild` (created automatically). On success, the binary is located at `./scriptbuild/src/yangl`:
```
./build.sh
```

If Qt is installed in a custom location (e.g. via Qt Online Installer), pass it explicitly:
```
./build.sh --qt-dir ~/Qt/6.8.3/gcc_64
```

<a name="toc-manual-steps"></a>
#### Manual steps

```
mkdir ./manualbuild
cd ./manualbuild
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

<a name="toc-packaging"></a>
## Packaging

<a name="toc-deb"></a>
### .deb (Debian/Ubuntu)

[build-deb.sh](build-deb.sh) builds the project and creates a `.deb` package in `./debbuild`. Requires `dpkg-deb`:
```
./build-deb.sh
```

<a name="toc-rpm"></a>
### .rpm (Fedora/RHEL/openSUSE)

[build-rpm.sh](build-rpm.sh) builds the project and creates an `.rpm` package in `./rpmbuild`. Requires `rpmbuild`:
```
./build-rpm.sh
```

On Fedora/RHEL: `sudo dnf install rpm-build`
On Debian/Ubuntu: `sudo apt install rpm`

<a name="toc-appimage"></a>
### AppImage

[build-appimage.sh](build-appimage.sh) builds the project and creates a self-contained `.AppImage` in `./appimagebuild`. It automatically downloads [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) and its Qt plugin on first run:
```
./build-appimage.sh
```

The resulting AppImage bundles Qt and all dependencies — users just download, `chmod +x`, and run.

<a name="toc-custom-qt-path"></a>
### Custom Qt path

All build scripts accept the `--qt-dir` option to specify a non-system Qt installation:
```
./build-deb.sh      --qt-dir ~/Qt/6.8.3/gcc_64
./build-rpm.sh      --qt-dir ~/Qt/6.8.3/gcc_64
./build-appimage.sh --qt-dir ~/Qt/6.8.3/gcc_64
```

When omitted, the scripts use whatever Qt6 is found on the system.

<a name="toc-for-developers"></a>
## For developers

<a name="toc-running-tests"></a>
### Running tests

Unit tests are off by default to keep user builds lean. Enable them with:

```
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
```

Then run them:
```
ctest --test-dir build --output-on-failure
```

<a name="toc-code-style"></a>
### Code style

The project enforces formatting via [clang-format](https://clang.llvm.org/docs/ClangFormat.html). CI will reject un-formatted code. To format locally:
```
clang-format -i $(git diff --name-only HEAD | grep -E '\.(cpp|h)$')
```
