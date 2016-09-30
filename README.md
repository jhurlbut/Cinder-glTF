# Cinder-glTF
glTF support for the Cinder framework

glTF is a runtime asset format for WebGL, 
OpenGL ES, and OpenGL. Read more [here](https://github.com/KhronosGroup/glTF)

##Features
- multiple materials from the same shader source (techniques vs materials)
- multiple meshes from the same buffer (accessors)
- transformation hierarchy
- animation channels
- GPU instancing

##Pipeline
- Author in 3D app (Maya, C4D, etc)
- export OpenCollada format with materials, meshes, and animation data
- use Collada2GLTF app to convert collada to gltf binary blobs, json descriptor and glsl shader source
- use this code to load json, images, shaders and binary data

