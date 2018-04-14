# teapotstein
##### an OpenGL FPS where you shoot teapots at teapots and the teapots shoot teapots back at you


## How to get this to run

OpenGL, GLU and Freeglut are required  
**UNIX**: compile with `g++ main.cpp Camera.h Camera.cpp Hitbox.h -o teapotstein -lGL -lglut -lGLU` and run with `./teapotstein`  
**Windows**: this repo contains packages.config, add the source files to a VS project and compile with the NuGet packages

## Controls

* WASD to move around
* Mouse to look around
* Left click to shoot
* Right click to fly (debugging feature, not part of gameplay)

## Gameplay

The following are functional:

* Full movement and camera control
* Shooting and damaging enemies
* Lighting and rendering simple objects
* Random enemy movement
* Skybox and floor(randomly tinted brown-greenish tiles) - this causes framerate drops for some reason

## TODO list

* Enemies shooting at the player and dealing damage to them
* Enemies moving towards the player
* Despawning old or dead objects (currently enemies and projectiles never disappear)
* Small visual effects (bullet trails, impact effects)
* Player health and HUD (currently the only part of the HUD implemented is the FPS counter)
* Make the world prettier with more objects


*this is a project for my C++ lab at MFF CUNI*
