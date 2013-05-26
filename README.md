Asteroids
=========

This document highlights the key commands necessary to start and play asteroids, as well as the features that were implemented. The game can be run in command line under the c file, “Asteroids.c”, using normal compiler and execution for OSX:

   	$ gcc -std=c99 -o Asteroids Asteroids.c -framework OPENGL -framework GLUT 
   
   	$ ./Asteroids
   
This game is a near replica of the arcade game Asteroids. It has some additional features and slightly different style but the gameplay remains the same; destroy the asteroids and move through the level without being hit. You have three lives to complete 8 levels. Each level has large asteroids which break into two medium sized ones each which in turn break into two small asteroids. Each level passed there is another asteroid added, which increases the challenge. 

  	Space: Fire a photon.
  	Up Arrow: Accelerate forward in the direction currently faced.
	Down Arrow: Accelerate backwards away from the direction currently faced.
	Left Arrow: Rotate the ship counter-clockwise.
	Right Arrow: Rotate the ship clockwise.

Will you be the one to defeat the evil asteroid empire once and for all?!?!
