# teapotstein
##### an OpenGL FPS where you shoot teapots at teapots and the teapots shoot teapots back at you


## How to get this to run

OpenGL, GLU and Freeglut are required  
**UNIX**: compile with `g++ main.cpp Camera.h Camera.cpp Hitbox.h Projectile.h Particle.h -o teapotstein -lGL -lglut -lGLU` and run with `./teapotstein`  
**Windows**: this repo contains packages.config, add the source files to a VS project and compile with the NuGet packages

## Controls

* Space to toggle between FPS controls and free mouse
* WASD to move around
* Mouse to look around
* Left click to shoot
* Right click to fly (debugging feature, not part of gameplay)
* ESC to close the game

## Gameplay

The following are functional:

* Full movement and camera control
* Shooting and damaging enemies
* Score counter for killing enemies
* Enemies shooting at the player
* Enemies moving towards the player and dealing contact damage
* Player HP & game over state on death
* Projectiles with a limited lifetime
* Hitboxes
* Lighting and rendering simple objects
* Random enemy movement
* Skybox and floor(randomly tinted brown-greenish tiles)
* Small visual effects (bullet trails, impact effects)

## TODO list

* Make the world prettier with more objects


*this is a project for my C++ lab at MFF CUNI*
