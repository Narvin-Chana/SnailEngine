# Snail Engine

DirectX 11 C++ Game Engine. 

Created by :
- Narvin Chana
- Alexandre Lemarbre-Barrett
- Paul Richard
- Hugo Pierron

Scenes are created via .json files like in Example.json.
Some default scenes with meshes, textures and other assets are available [here.](https://drive.google.com/file/d/10jespw39jNmwH45xIiqIbtw1k69LX8vv/view?usp=sharing)

## Features
- Blinn-Phong deferred lighting
- Screen-Space Deferred Decals
- Directional Light (CSM with PCF)
- Spot and Point Lights
- Normal Mapping
- Volumetric Lighting inspired by [Benjamin Glatzel's work](https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen)
- Procedural GPU-Instanced Grass inspired by [Sucker Punch's work on Ghost of Tsushima](https://www.gdcvault.com/play/1027214/Advanced-Graphics-Summit-Procedural-Grass)
- Terrain Loading and Chunking
- Multithreaded .obj parsing and asset management
- SSAO
- FXAA
- PhysX 5.3 Integration
- ImGui Editor (Scene JSON loading and serialization)

## Screenshots

The following screenshots are all taken in-engine.

### Screen-Space Deferred Decals

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/d3105520-d400-4d94-977a-da714201567e)

### Volumetric Lighting

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/184b7e79-053d-4590-be37-cf15bda1e9bb)
![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/d71fcff9-30f2-4afd-8837-fba12f9cc9bb)

Here is how it is achieved in real time with potentially many lights :

1) Initial accumulation Pass with interleaving at half-resolution

| Accumulation Buffer (and zoom) |
|---| 
| ![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/68dfee1c-11e5-4250-8dd9-2156c5962612) |

2) Bilateral Gaussian Blur Pass to reduce noise (and conserve shape)

| Before Blur Pass | After Blur Pass |
|---|---|
| ![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/c19dce0d-7367-46c1-8c79-4bc03aa4bd58) | ![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/1bec98de-21f7-40e5-9f7f-cffe05205344) |

3) Nearest-Depth Upsample Pass

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/83e97a9c-dfca-4c8e-8923-6fa0b57f103c)

### Procedural Grass (GPU Instanced)

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/778fd42a-e136-492b-b912-48785c0e2fe8)

Instance Data Generated in Compute Shader :

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/06225ff9-4bca-45b3-9240-fad10350cbd9)

Texture Masking :

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/b91b66f8-8fb2-4be5-9928-3b5e33324a40)

### SSAO

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/0c55abc7-e2bb-459b-9084-b1b1790710f8)

### FXAA

![image](https://github.com/Narvin-Chana/SnailEngine/assets/36044215/b43c9de0-375c-45da-9c74-f390910988a0)
