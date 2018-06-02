# Lavos

[![Build Status](https://travis-ci.com/thestr4ng3r/lavos.svg?branch=master)](https://travis-ci.com/thestr4ng3r/lavos)

Lavos is a 3D engine framework based on the Vulkan graphics API.

It aims to have a more modular architecture than other engines, such as Unity.
The goal is to provide a collection of components, such as different kinds of renderers and materials,
that can be mixed together to produce exactly the kind of engine that is needed for an application
while being lightweight and not adding anything unnecessary.

**Lavos is still in an early development stage. Consider everything experimental and work in progress!**

## Shells

Different libraries, called "shells", for easily integrating Lavos in an environment exist.
The shells currently available are for **GLFW** and **Qt**.

## Cloning this Repository

The demo assets are stored using Git LFS.
If you would like to run the demos, make sure you have it installed: https://git-lfs.github.com/.

Third party libraries and demo assets are integrated into this repository
as git submodules, so use `--recursive` when cloning:

```
git clone --recursive https://github.com/thestr4ng3r/lavos.git
```

If you already cloned the repository without the submodules,
run the following command inside of it:

```
git submodule update --init --recursive
```


## Building

Lavos uses CMake as its build system. It can thus be built like this:

```
mkdir build && cd build
cmake ..
make
```

A number of options can be specified to cmake like `cmake -DOPTION=ON/OFF ..`:

| Option                       | Default | Description |
| ---------------------------- | ------- | ----------- |
| LAVOS_BUILD_DEMOS            | ON      | Build Demos |
| LAVOS_BUILD_POINT_CLOUD_DEMO | OFF     | Build Point Cloud Demo (requires PCL) |
| LAVOS_BUILD_QT_DEMOS         | OFF     | Build Qt Demos |
| LAVOS_BUILD_SHELL_GLFW       | ON      | Build GLFW Lavos Shell library |
| LAVOS_BUILD_SHELL_QT         | OFF     | Build Qt Lavos Shell library |
| LAVOS_IMPLEMENT_STB_IMAGE    | ON      | Compile implementation of stb_image inside Lavos. |
| LAVOS_IMPLEMENT_TINYGLTF     | ON      | Compile implementation of tinygltf inside Lavos. |
| LAVOS_IMPLEMENT_VMA          | ON      | Compile implementation of Vulkan Memory Allocator inside Lavos. |

## Documentation

Doxygen documentation for the code can be built by running
`doxygen` inside the [docs](docs) directory.
Documentation is still very sparse, but it will be extended continuously.

## About

Created by Florian Märkl

Lavos is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Lavos is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
