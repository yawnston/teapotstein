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
* Right click to fly (debugging feature, not part of gameplay! enemies get very confused by the player flying)
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

## Programmer's notes

The whole thing is built in pure OpenGL, using GLUT to do some of the window management stuff. It uses the fixed pipeline.  
Everything is mostly in C, with some C++ features like classes/containers.  
Drawing is handled inside the display() function, which is called by glutIdleFunc and glutDisplayFunc. All code that draws anything on the screen is transitively called by this function.  
Update is done by glutTimerFunc callbacks. Again, all movement/update code is called transitively from these callbacks.  
Input is parsed using glut callbacks, with a interframe buffer for storing the input inbetween update calls.  
Parameters of gameplay like projectile speed or health of things are defined as constants either in their respective header files, or directly in the main file. This allows for easy tuning of gameplay. Too easy? Reduce player health, increase enemy speed or recude player invulnerability frames!  
Lighting is done by a single directional and a single ambient light source.  
Rendering of specific items is either done using glut teapots or manual vertices.  
Projectiles have a frame lifetime after which they are removed. They're stored in a deque for easy adding/removal. This means that sometimes, a "dead" projectile might be stuck for a little bit longer inside the deque, because projectiles with a longer lifetime might be stuck in front of it. Removing these projectiles would be too expensive, because it would require reorganizing the container, so they're just skipped over during rendering until the projectiles in front also die. The performance hit of this is not significant, because effort is made to make sure that projectiles in the deques are more or less in a sorted order by lifetime, which means that most dead projectiles can be found be just removing the front elements.  


*this is a project for my C++ lab at MFF CUNI*
