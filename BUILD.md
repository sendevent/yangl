# yangl build

The general description of build process. 

## Getting Started

I assume you already have the c++ build tools and git installed and the source code is cloned to your local file system. Or you do know how to perform that in used environment :)

```
sudo apt install git build-essential # may depend on your Linux distro
mkdir ~/yangl # the actual dir name does not matter
cd ~/yangl
git clone https://github.com/sendevent/yangl.git .
```

## Prerequisites
### Qt
#### Version
6.8+

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

### C++

The most-modern-as-for-today (c++23-capable) compiller — gcc 14.2.0 or clang 19.1.7 would be enough.

## Building

### QtCreator

Open [CMakeLists.txt](CMakeLists.txt) and build it as a regular project.

### Shell

#### Build script

[build.sh](build.sh) performs the build process in the sub directory `./scriptbuild` (created automatically). On success, the binary is located at `./scriptbuild/src/yangl`:
```
./build.sh
```

If Qt is installed in a custom location (e.g. via Qt Online Installer), pass it explicitly:
```
./build.sh --qt-dir ~/Qt/6.8.3/gcc_64
```

#### Manual steps

```
mkdir ./manualbuild
cd ./manualbuild
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Packaging

### .deb (Debian/Ubuntu)

[build-deb.sh](build-deb.sh) builds the project and creates a `.deb` package in `./debbuild`. Requires `dpkg-deb`:
```
./build-deb.sh
```

### .rpm (Fedora/RHEL/openSUSE)

[build-rpm.sh](build-rpm.sh) builds the project and creates an `.rpm` package in `./rpmbuild`. Requires `rpmbuild`:
```
./build-rpm.sh
```

On Fedora/RHEL: `sudo dnf install rpm-build`
On Debian/Ubuntu: `sudo apt install rpm`

### AppImage

[build-appimage.sh](build-appimage.sh) builds the project and creates a self-contained `.AppImage` in `./appimagebuild`. It automatically downloads [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) and its Qt plugin on first run:
```
./build-appimage.sh
```

The resulting AppImage bundles Qt and all dependencies — users just download, `chmod +x`, and run.

### Custom Qt path

All build scripts accept the `--qt-dir` option to specify a non-system Qt installation:
```
./build-deb.sh      --qt-dir ~/Qt/6.8.3/gcc_64
./build-rpm.sh      --qt-dir ~/Qt/6.8.3/gcc_64
./build-appimage.sh --qt-dir ~/Qt/6.8.3/gcc_64
```

When omitted, the scripts use whatever Qt6 is found on the system.

## For developers

### Running tests

Unit tests are off by default to keep user builds lean. Enable them with:

```
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
```

Then run them:
```
ctest --test-dir build --output-on-failure
```

### Code style

The project enforces formatting via [clang-format](https://clang.llvm.org/docs/ClangFormat.html). CI will reject un-formatted code. To format locally:
```
clang-format -i $(git diff --name-only HEAD | grep -E '\.(cpp|h)$')
```
