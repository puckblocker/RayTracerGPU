# RayTracerGPU

![Example Render: Cornell Box](100kCornell.jpg)

This is a hardware-accelerated path tracer built with C++ and OpenGL 4.6 compute shaders. It provides a highly flexible rendering environment, allowing users to render custom scenes, import 3D geometry, and dial in physically based materials.

## Features

*   **PBR Materials:** Full support for rough and smooth dielectrics, specular surfaces, and conductors.
*   **Complex Light Transport:** Capable of rendering thin-layered BRDFs.
*   **Dynamic Cameras:** Includes motion blur and depth of field parameters.
*   **Diverse Lighting:** Support for area, point, directional, and emissive geometry lights.
*   **Custom Geometry:** Direct loading of .obj file geometry.

## Usage & Installation

**For Standard Users:**
The easiest way to use the ray tracer is to navigate to the **Releases** tab on the right side of this repository. Download the latest `.zip` release, extract it, and run the executable. The built-in graphical user interface will guide you through loading scenes and rendering.

**For Developers (Building from Source):**
If you wish to compile the engine yourself, pull the repository and build the source files using standard C++ build tools (like CMake). Ensure your environment is configured for OpenGL and your graphics drivers are up to date.

## Formatting .OBJ Files

To ensure geometry loads and renders correctly, all `.obj` files must be exported as **Triangulated Meshes** from your 3D software (quads are not currently supported by the parser).

If you want to customize the lighting or camera setup for a specific model, you can paste the text parameters from the `StandardScene` file directly into the top of your `.obj` text file and tweak them to your preference!

## Known Issues

*   **Operating Systems:** Linux environments are not currently supported.

## Future Plans

*   Implement proper Texture Mapping for imported models.
*   Integrate Bounding Volume Hierarchies (Bounding Boxes / KD Trees) to optimize complex mesh intersections.
*   Add internal Tone Mapping for improved exposure control.
*   Reintroduce Spectral Rendering capabilities.
*   Implement a physical Power/Wattage multiplier stat for light sources.
*   Add Volumetric rendering (fog/scattering).
*   Expand the engine to handle animated sequences.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
