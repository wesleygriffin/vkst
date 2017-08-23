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

Either do a `git clone --recurse-submodules` or a
`git submodule update --init` after cloning to update.

CMake should take care of configuring and building the submodules.

## Building

Currently only Visual Studio 15.3 has been tested. There is a bit of code to
add and then Linux support should be available.

## Code

I have used std::vector in certain places in the code. In most of these cases,
an array would work or, even better, a stack-allocated vector.

I have also tried to use std::error_code everywhere to report errors instead
of using exceptions. I have wrapped the existing VkResult and shaderc error
codes to allow them to be coded in a std::error_code.

The three pieces of Vulkan code are in:
- `renderer::create`
- `renderer::create_surface`
- `renderer::resize_surface`

these functions do all of the work to initialize and create the renderer.

You can see how these functions are called in st.cc::init which also
allocates command buffers for drawing and builds the pipeline.
