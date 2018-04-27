# Computer-Graphics

## How we found the main coding project
really enjoyed, so cool to be creating 3D graphics.

Had loads of cool ideas and enjoyed challenges ____.





## Features we've included (with pictures?)
* code organised into modules
* Homogenous coordinates for camera movement
  - moving things in cameraspace?
  
### raytracer
* specular things
  - line 364
* parrallelisation
  - line 223
* colour modes
  - line 185, line 243
  
### rasteriser
* 


## Features we've worked on but not finished (with pictures?)
* gobo in raytracer
  - or call it shadow mapping?
  - plan 1:
    * build gobo array from file, check where light ray goes through on a nxn square that's 0.1 below the light source then map this to 100x100 grid that is the gobo.
    * ended up with horizontal rings of light on the walls near the top.
    * eventually gave up on this and tried again from scratch.
  - plan 2:
    * created a cone of light first that comes to about 1/3 of the way down the walls by testing cos(theta)
    * managed to get a grainy/spotty circle on the floor, but this light wasn't projected onto the blocks
    * didn't really think where to go from then, probably revisit plan 1
* stencil shadows in rasteriser



## Final thoughts/what we would've liked to work on.
* world object
* fire
  - step1: implement objects moving
  - step2: generate "particles" (tiny triangles) that are created at one point then move 0.1 then dissapear
  - step3: code opacity
  - step4: have particles linearly lose opacity
  - add randomness so they don't all move the same distance/lose opacity linearly/half lifes
  - light??
  - smoke??
* fog/haze
* clipping
