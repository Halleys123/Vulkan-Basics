This folder only contains raw assets which when passed through appropriate asset pipeline will be converted to correct format for example JPG to BC7.

In Shaders folder, to allow makefile to run correctly for max nesting in this folder will be two.

All the compiled assets will also be first placed in this folder then moved onto next folder, all compiled assets will be placed in same folder @root/assets/compiled in similar folder order i.e.

If raw assets are as follows

```
root/assets
|-shaders
  |--vertex
    |-box.vert
    |-apple.vert
  |-frag
    |-box.frag
    |-apple.frag
|-textures
  |-file1.jpg
  |-file2.png
```
then compiled assets should be placed in 

```
root/assets/compiled
|-shaders
  |--vertex
    |-box.spv
    |-apple.spv
  |-frag
    |-box.spv
    |-apple.spv
|-textures
  |-file1.bc7
  |-file2.bc7
```

Compiled folder should not be commited
