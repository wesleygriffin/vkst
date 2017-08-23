# vkst - Vulkan-based ShaderToy Example

This is a basic sample of using Vulkan to create a small application that
draws a full screen quad and can run fragment shaders from ShaderToy.

Currently only shaders that don't use textures are supported.

## Dependencies

There are four git submodules in ext/ that are required for building:
- [gsl-lite](https://github.com/martinmoene/gsl-lite)
- [turf](https://github.com/preshing/turf)
- [shaderc](https://github.com/google/shaderc)
- [glm](https://github.com/g-truc/glm)

shaderc is used to enable live shader recompiles (more on that below).

## Building

Currently only Visual Studio 15.3 has been tested. There is a bit of code to
add and then Linux support should be available.
