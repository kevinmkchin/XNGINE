## Development Log

### 2021-07-07
- Light culling based on 16x16 tiles on the screen. If the tile is within range of 
  a point light, then for the fragments within that tile, we will accumulate light 
  from that point light. Variable performance - around ~90 to ~100 fps. Dependent on
  the scene, but overall much improved from forward rendering as well as regular
  deferred rendering.

### 2021-07-04 - 2021-07-07
- Lighting calculations and shading moved into compute shader. Still no light culling
  but performance improved from ~45 fps in no compute shader deferred renderer to ~55 fps
  in compute shader deferred renderer.

### 2021-07-03
- Deferred renderer with no light culling (no optimization)

### 2021-06-26 - 2021-07-01
- Mass rewrite to new engine architecture - from single transation unit build to separate source files

### 2021-06-25
- Shadows for multiple point lights and spot lights
- Omni-directional shadows for point lights and spot lights

### 2021-06-24
- Fix error in camera.cpp where FOV needed to be in radians not degrees.

### 2021-06-20
- Directional shadows rendering implementation - needs refactoring and clean up
- Counter-clockwise vertex winding order for triangles created in kc_truetypeassembler.h
- Keeping a CHANGELOG starting this day