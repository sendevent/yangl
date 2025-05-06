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

[build.sh](build.sh) performs the build process in the sub directory ./scriptbuild (created automatically). On success, the directory would contain yangl executable:
```
chmod +x ./build.sh && ./build.sh
```

#### Manual steps

```
mkdir ./manualbuild
cd ./manualbuild
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j`nproc`
```
