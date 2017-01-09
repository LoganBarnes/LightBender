
Light Bender
============

Testbed for NVIDIA OptiX

![](./renderings/xwing/x-wing_both.ppm)


Download
--------
Make sure to clone all submodules as well:

```bash
git clone --recursive https://github.com/LoganBarnes/SubtractiveTerrain.git
```

If the project was already cloned without submodules use this command from the root folder:

```bash
git submodule update --init --recursive
```


Unix
----

Ensure the Vulkan SDK location is added to the environment paths before running commands.

```bash
cd run
./unixConfigureAndBuild.sh
./bin/runSubtractiveTerrain
```


Windows
-------

Ensure the Vulkan SDK location is added to the environment paths and Visual Studio has been installed before running commands.

```bash
cd run
winConfigureAndBuild.cmd
bin\runSubtractiveTerrain.exe
```


Manually via CMake (platform independent)
------------------------------------------

Ensure the Vulkan SDK location is added to the environment paths before running commands.

```bash
cd run
mkdir _build
cd _build
cmake ../..
cmake --build . --config Release
```

This will create an executable in the ```run/_build``` folder or in the ```run/_build/Release``` folder depending on the local build setup.

