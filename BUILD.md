# yangl build

The general description of build process. 

## Getting Started

I assume you already have the c++ build tools and git installed and the source code is cloned to your local file system. Or you do know how to perform that in used environment :)

```
sudo apt install git build-essential
mkdir ~/yangl
cd ~/yangl
git clone https://github.com/sendevent/yangl.git .
```

## Prerequisites
### Qt
#### Version
Should be compilable with >= 5.7, but you may need to tune versions of used qml imports in [MapView.qml](app/geo/qml/MapView.qml)
While tested with 5.12, the primary development version for now is 5.15.

#### Modules

* base (core, gui, widgets, concurrent)
* qml (quick, quickwidgets)
* geo (location, positioning)

Here's how to install these on Debian:

```
sudo apt install qt5-default \
                qtbase5-dev \
                qtdeclarative5-dev \
                qtquickcontrols2-5-dev \
                qtlocation5-dev \
                qtpositioning5-dev
```

Please refer to your distro manual to get the related packages.

### C++

Relatively modern (c++14-capable) compiller — gcc 9.3 or clang 8 would be enough.

## Building

### QtCreator

Open [yangl.pro](yangl.pro) and build it as a regular project.

### Shell

#### Build script

[build.sh](build.sh) performs the build process in the sub directory ./build (created automatically). On success, the directory would contain yangl executable:
```
chmod +x ./build.sh && ./build.sh
```

#### Manual steps

```
mkdir ./build
cd ./build
qmake -r ../yangl.pro #or qmake-qt5, depending on your Linux distro
make -j`nproc`
```
