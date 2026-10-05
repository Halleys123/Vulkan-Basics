# Information

This is a repository created just so that I can learn about vulkan and save some of the important process that is reused in many projects.

## Current collected items

### `Build Process`

By mixing use of CMake and Makefile, I can allow me/others to simply run `make run` command irrespective of OS to build the application for their OS. This makefile can also handle creation of `compile_commands.json` on windows for users who are not using Visual Studio building the application (and are rather using VSCode or Zed for better intellisense). 

### `Pipelines`

The following two are just example of assets there could be many other types of assets, but for sake of explanation I selected these two as these are better for explanation.

#### `Textures`
Many of the assets that are used in games are in specialized formats for better GPU efficiency and Bandwidth consumption, that means JPG or PNG like formats are not suited for games as they take too much GPU RAM, rather formats like BC7 are used for this purpose, but the texture formats most of the files we can find online are found in JPG, PNG or other image formats, but those are not good for our game.

#### `Shaders`
Similarly Vulkan requires shaders to be already in the compiled format that is SPIR-V rather than GLSL or any other format, but most of the programmers use glsl and we can't write binary format by hand (or is very difficult and time consuming so not good way to go).

### Solution
So the solution is get the textures in the format they are available, write the shaders in format most people are suited for, then we can write a program that will convert textures to required format, compile the shaders before we run the main program. Now after conversion to suitable format is finished we can use them in main program, now a diellema arises these converters are not needed by people who use our application (or gamers) that is because we would have already compiled the final chosen shaders and texture to required format and shipped with application. So that is why these programs are rather called with a special name that is `pipelines`.

So a `pipeline` or `pipeline program` is a program that is used for helping with development but not shipped with the final application.

#### Common source of misunderstandings

1. Question:  Why not just compile these once assets even during development time rather than creating these pipelines?
1. Answer: That is becuase these assets may change during every once or during testing so rather than recompiling shaders or converting textures again and again, we make that a process of build itself, so that it can be automated.

2. Question: Are there only two types of assets (not important but still for people who are new)
2. Answer: No these are not the only assets that can be there in program, other assets may include 3D Models or Sound, VFX etc.

# Folder Structure

1. `externals` - This folder contains libraries static or dynamic that are third party and will be used by my application.
2. `libs` - Libraries that I will make in the project and will be used by the application which can be static or dynamic.
3. `include` - Just include header files
4. `src` - Implementation files
5. `assets` - Assets like textures or shaders in the raw format, which on building main application will then be converted to final format.
6. `pipelines` - The build pipelines that run during development and are not intended to be shipped with the final build.

## Future Information

When any Vulkan project grows there I will use TODO inside code, but TODO can be of multiple types like TODO:OPTIMIZATION maybe like O(n^2) search to some other better algorithm, which can be further divided into types like `TODO:OPTIMIZATION:ALGO` or `TODO:OPTIMIZATION:COMPILER` for further clarification each todo can be modified to use `TODO:OPEN:OPTIMIZATION:ALGO` or `TODO:CLOSED:OPTIMIZATION:ALGO` or `TODO:WORKING:OPTIMIZATION:ALGO`

List of other TODO types that I can use in future are as following (NEED TO ADD)
