# Lightcuts

This project is an implementation of the lightcuts algorithm

## Howto

### Install

This project depends on:
- C++20
- Vulkan
- glfw
- OpenMP

Make sure to have these dependencies installed on your machine before installing the project. 

On Arch:
```sh
sudo pacman -S vulkan-validation-layers glfw gcc openmp
```
On Ubuntu:
```sh
sudo apt install vulkan-utils libglfw3 libglfw3-dev gcc libomp-dev
```
On Fedora:
```sh
sudo dnf install vulkan-validation-layers glfw-devel gcc libomp-devel
```

To install the project, clone the github repository:
```sh
git clone --recurse-submodules --remote-submodules https://github.com/MrBigoudi/LightCuts.git
```
If the folder `LightCuts/src` doesn't contain the `engine` subfolder (might happen because of a github submodule issue), run the following:
```sh
cd LightCuts/src
git clone https://github.com/MrBigoudi/BigoudiEngine.git
mv BigoudiEngine engine
cd ..
```

### Usage

To use the project run the following commands from the `LightCuts` repository:
```sh
# for the debug mode
# cmake -B build
# for the release mode
cmake -B build -DCMAKE_BUILD_TYPE=Release

make -C build
./build/lightcuts
```

When the application starts, you see the scene displayed through the rasterizer. The loaded scene contains a small amount of lights to not make the rasterizer too laggy. 
However, two rendering modes are available: a Rasterizer and a Raytracer.

Rasterizer:
Available actions in rasterizer mode are:

- `W`, `A`, `S`, `D`, `Up`, `Down`: press these keys to move the camera in the scene

- `Mouse`: use your mouse to rotate the camera (only works when the camera mode is on)

- `M`: press `M` to activate or deactivate the camera mode. In camera mode, you can rotate the camera with the mouse. When deactivated the view is locked to make the usage of the ImGui window more convenient

- `F1`: press `F1` to activate the wireframe mode

- `P`: press `P` to switch the shader to use. There are 5 possible shaders: a color passthrough to display albedo colors, a normal passtrough to display the normals in world space, a Lambert BRDF, a Blinn-Phong BRDF, a Microfacet BRDF and a Disney BRDF.

- `ImGui`: the properties in the ImGui window control the material of the object at the center of the scene (either the sphere or the dragon). Note that these values are not used in all the possible shaders

- `Tab`: press `Tab` to switch to Raytracer mode


Raytracer:
Available actions in raytracing mode are:

- `Space`: press `Space` to run the raytracer (be carefull this action can take some time). Informations are displayed on the terminal to let you know the advancement of the raytracer

- `ImGui`: the properties in the ImGui window control the raytracer. 

```
    -Save result image: save the resul image as a ppm in the current directory</li>
    -Samples per pixels: the number of ray per pixels in the image (for subpixel rendering)</li>
    -Max bounces: the number of allowed bounces for the path tracer (default to 0 for the raytracer)</li>
    -Samples per bounces: the number of randomly cast rays after each bounce</li>
    -Shading factor for bounces: the factor by which the color reponse after each bounce should be multiply by</li>
    -Use lightcuts: use the lightcuts algorithm or not</li>
    -Error threshold: the lightcuts error threshold (default 2%)</li>
    -Minimum intensity: the minimum intensity for a light (for dark regions)</li>
    -Maximum size of a cut: the maximum number of cluster per cuts</li>
```

- `Tab`: press `Tab` to switch to Rasterizer mode
