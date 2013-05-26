/*
 *	asteroids.c
 *  Skeleton Code Written By: Dirk Arnold
 *  Additional Features and Full Function Written By: Ryan MacLeod
 *  Last Date Update: October 15, 2012
 *
 *  This game is a near replica of the arcade game Asteroids. It has so additional features
 *  and slightly different style but the gameplay remains the same; destroy the asteroids and
 *  move through the level without being hit. You have three lives to complete 8 levels. Each
 *  level has large asteroids which break into two medium sized ones each which in turn break
 *  into two small asteroids. Each level passed you add another asteroid which increases the
 *  challenge. Will you be the one to defeat the evil asteroid empire once and for all?!?!
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define myTranslate2D(x,y) glTranslated(x, y, 0.0)
#define myScale2D(x,y) glScalef(x, y, 1.0)
#define myRotate2D(angle) glRotatef(RAD2DEG*angle, 0.0, 0.0, 1.0)

#define SHIP_VERTICES 3
#define MAX_PHOTONS	8
#define MAX_LARGE_ASTEROIDS 8
#define MAX_ASTEROIDS 32
#define MAX_VERTICES 16
#define MAX_STARS 50
#define MAX_DUST 32
#define DUST_PARTICLES 15

#define TIME_WAIT 50

#define SHIP_VELOCITY_MAX 2.0
#define ACCELERATION_STEP_FORWARD 0.1
#define ACCELERATION_STEP_BACK -0.1

#define LARGE_SIZE 3.0
#define MEDIUM_SIZE 2.0
#define SMALL_SIZE 1.0

/* -- type definitions ------------------------------------------------------ */

typedef struct Coords {
	double		x, y;
} Coords;

typedef struct {
    int engine;
	double	x, y, phi, dx, dy;
    Coords coords[SHIP_VERTICES];
} Ship;

typedef struct {
	int	active;
	double	x, y, dx, dy;
} Photon;

typedef struct {
	int	active, nVertices;
	double	x, y, phi, dx, dy, dphi, size;
	Coords	coords[MAX_VERTICES];
} Asteroid;

typedef struct {
    Coords coords[4];
} StartBox;

typedef struct {
    double x, y;
} Stars;

typedef struct {
    Coords coords[DUST_PARTICLES];
    int active;
    int dustTimer;
    int drawThisFrame;
} Dust;

/* -- function prototypes --------------------------------------------------- */

// Collision Detectors
static int PhotonCollision(Photon *p, Asteroid *a);
static int ShipCollision(Coords *c, Asteroid *a);

// Display Callbacks for the Three Screens
static void	myGameDisplay(void);
static void	myMenuDisplay(void);
static void myLevelDisplay(void);
static void levelMyDisplay(void);


// Time Callbacks for the Three Screens
static void	gameMyTimer(int value);
static void	menuMyTimer(int value);
static void levelMyTimer(int value);
static void gameOverMyTimer(int value);


// Callback functions for the keyboard and mouse
static void	myKey(unsigned char key, int x, int y);
static void	keyPress(int key, int x, int y);
static void	keyRelease(int key, int x, int y);
static void mouseClick(int button, int state, int x, int y);

static void	myReshape(int w, int h);

// Initialize functions for the menu and game sections
static void	gameInit(void);
static void	menuInit(void);

// Initializes random asteroids of varying shapes and sizes.
static void	initAsteroid(Asteroid *a, double x, double y, double size);

// Functions to draw the specific objects related to the game.
static void	drawShip(Ship *s);
static void drawLives(int lives);
static void drawText(char *, void * font, double x, double y);
static void drawDust(Dust *dust);
static void	drawPhoton(Photon *p);
static void	drawAsteroid(Asteroid *a);
static void drawMenu(void * font);
static void drawRotatingShip();
static void drawStars(Stars *stars);

// Helper classes to be used with the program.
static double myRandom(double min, double max);
static int withinBox(double x, double y, StartBox *box);
static int findInactiveAsteroid();
static int findInactivePhoton();
static void updateVelocity(int state);
static int levelBeat();
static char * getLevelNumber();
static void activateDust(double x, double y);
static void activateExplosion(double x, double y);

/* -- global variables ------------------------------------------------------ */

static int	up=0, down=0, left=0, right=0;	/* state of cursor keys */
static double	xMax, yMax;

// Objects to be drawn in side the coordinate system.
static Ship	ship;
static Photon	photons[MAX_PHOTONS];
static Asteroid	asteroids[MAX_ASTEROIDS];
static StartBox startbox;
static Stars stars[MAX_STARS];
static Dust dust[MAX_DUST];
static Dust shipExplosion;

// Help control the state of the game and certain animations
static int lives = 3;
static int otherFrame = 0;
static int gameState = 0;
static int betweenLevelTimer = 0;

/* -- main ------------------------------------------------------------------ */

int
main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("Asteroids");
    
    glutDisplayFunc(myMenuDisplay);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(myKey);
    glutSpecialFunc(keyPress);
    glutSpecialUpFunc(keyRelease);
    glutReshapeFunc(myReshape);
    glutMouseFunc(mouseClick);
    glutTimerFunc(33, menuMyTimer, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    menuInit();
    
    glutMainLoop();
    
    return 0;
}

/* -- callback functions ---------------------------------------------------- */

/* This callback functions displays all necessary objects for the menu. It includes
 * a start button that is only active in this state.
 */
void
myMenuDisplay(){
    /*
     *	display callback function for menu
     */
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the point size of the stars.
    glPointSize(2.0);
    
    // Draw the stars in the back.
    for(int i = 0; i < MAX_STARS; i++){
        glLoadIdentity();
        drawStars(&stars[i]);
    }
    
    // Reset the point size back to 4.0 for the photon shots.
    glPointSize(4.0);
    
    // Load and store the identity matrix twice for use with the photons and asteroids.
    glLoadIdentity();
    
    // Draw out the asteroids.
    for(int i = 0; i < MAX_ASTEROIDS; i++){
        if(asteroids[i].active){
            glLoadIdentity();
            myTranslate2D(asteroids[i].x, asteroids[i].y);
            myRotate2D(DEG2RAD*asteroids[i].phi);
            drawAsteroid(&asteroids[i]);
        }
    }
    // Draw the menu out in helvetica 18.
    glLoadIdentity();
    drawMenu(GLUT_BITMAP_HELVETICA_18);
    
    // Draw a ship circling around the start button.
    glLoadIdentity();
    myTranslate2D(90, 49);
    drawRotatingShip();
    
    glutSwapBuffers();
}

/* This simple display callback is used for a certain period of time in between levels
 * so that the player can prepare themselves. It displays the stars and the level number.
 */
void
myLevelDisplay(){
    /*
     *	display callback function for level main screen
     */
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the point size of the stars.
    glPointSize(2.0);
    
    for(int i = 0; i < MAX_STARS; i++){
        glLoadIdentity();
        drawStars(&stars[i]);
    }
    
    // Reset the point size back to 4.0 for the photon shots.
    glPointSize(4.0);
    
    glLoadIdentity();
    
    char* levelNumber = getLevelNumber();
    
    // Draw the level number.
    drawText(levelNumber, GLUT_BITMAP_HELVETICA_18, 77, 50);
    
    glutSwapBuffers();
}

/* This simple display callback is used when the player loses or when the player beats
 * level 8 of the game.  Only remains displayed for a set number of frames before reverting
 * back to the main menu.
 */
void
gameOverDisplay(){
    /*
     *	display callback function for game over screen
     */
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the point size of the stars.
    glPointSize(2.0);
    
    // Draw the stars.
    for(int i = 0; i < MAX_STARS; i++){
        glLoadIdentity();
        drawStars(&stars[i]);
    }
    
    // Reset the point size back to 4.0 for the photon shots.
    glPointSize(4.0);
    
    glLoadIdentity();
    
    char* gameOver = "GAME OVER!!";
    
    // Draw the game over text.
    drawText(gameOver, GLUT_BITMAP_HELVETICA_18, 77, 50);
    
    glutSwapBuffers();
}

/* This is the display callback used when the player is in level. It contains
 * drawing the ship, lives, level title, asteroids, explosions, stars, and dust.
 */
void
myGameDisplay()
{
    /*
     *	display callback function for game
     */
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the point size of the stars. 
    glPointSize(2.0);
    
    // Draw the stars.
    for(int i = 0; i < MAX_STARS; i++){
        glLoadIdentity();
        drawStars(&stars[i]);
    }
    
    // Reset the point size back to 4.0 for the photon shots.
    glPointSize(4.0);
    
    glLoadIdentity();
    
    // Draw the ship on screen or an explosion if they have been hit.
    myTranslate2D(ship.x, ship.y);
    myRotate2D(DEG2RAD*ship.phi);
    if(shipExplosion.active == 1){
        drawDust(&shipExplosion);
    }else{
        drawShip(&ship);
    }
    
    
    // Draw the photons if they are active.
    for (int i = 0; i < MAX_PHOTONS; i++){
    	if (photons[i].active){
            glLoadIdentity();
            drawPhoton(&photons[i]);
        }
    }
    
    // Draw the asteroids if they are active.
    for (int i = 0; i < MAX_ASTEROIDS; i++){
    	if (asteroids[i].active){
            glLoadIdentity();
            myTranslate2D(asteroids[i].x, asteroids[i].y);
            myRotate2D(DEG2RAD*asteroids[i].phi);
            drawAsteroid(&asteroids[i]);
        }
    }
    
    // Draw the dust from any previous explosions and hangle its timers and flicker.
    for (int i = 0; i < MAX_DUST; i++){
        if(dust[i].active){
            glLoadIdentity();
            if(dust[i].drawThisFrame){
                drawDust(&dust[i]);
            }
        }
    }
    // Draw the number of the level in which the player is currently playing.
    glLoadIdentity();
    drawText(getLevelNumber(), GLUT_BITMAP_HELVETICA_18, 10, yMax-6);

    /* Draw the lives text and the ships that represent each life left
     * to the player */
    glLoadIdentity();
    drawText("LIVES - ", GLUT_BITMAP_HELVETICA_18, xMax-30, yMax-6);
    for(int i = 0; i < lives; i++){
        glLoadIdentity();
        myTranslate2D(xMax-(5*i)-5, yMax - 5);
        drawLives(lives);
    }
    
    glutSwapBuffers();
}

/* The timer call back for the level last as long as the time wait macro
 * is specified for. It does not have anything to update other than the timer.
 */
void
levelMyTimer(int value){
    /*
     * Timer callback for screen between the levels.
     */
    
    glutPostRedisplay();
    
    if(betweenLevelTimer < TIME_WAIT){
        betweenLevelTimer = betweenLevelTimer + 1;
        glutTimerFunc(33, levelMyTimer, 0);
        
    }else{
        // Reset the between level timer.
        betweenLevelTimer = 0;
        // Reset the asteroids
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            asteroids[i].active = 0;
        }
        if(gameState > 8){
            glutTimerFunc(33, gameOverMyTimer, 0);
            glutDisplayFunc(gameOverDisplay);
        }else{
            glutTimerFunc(33, gameMyTimer, 0);
            glutDisplayFunc(myGameDisplay);
            gameInit();
        }
    }
}

/* The timer call back for the game over last as long as the time wait macro
 * is specified for. It does not have anything to update other than the timer.
 */
void
gameOverMyTimer(int value){
    /*
     * Timer callback for screen between the levels.
     */
    
    glutPostRedisplay();
    
    if(betweenLevelTimer < TIME_WAIT){
        betweenLevelTimer = betweenLevelTimer + 1;
        glutTimerFunc(33, gameOverMyTimer, 0);
    }else{
        // Reset the between level timer.
        betweenLevelTimer = 0;
        // Reset the asteroids
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            asteroids[i].active = 0;
        }
        menuInit();
        glutTimerFunc(33, menuMyTimer, 0);
        glutDisplayFunc(myMenuDisplay);
        gameState = 0;
    }
}

/* The menu timer call back is used to move the asteroids around the screen and 
 * to update the flicker of the ships engines. It changes the callback functions
 * when the start button is pressed.
 */
void
menuMyTimer(int value){
    /* 
     * Time callback function for menu.
     */
    
    // Change this each frame to give a flicker animation effect on affected objects
    if(otherFrame > 2){
        otherFrame = 0;
    }else{
        otherFrame = otherFrame + 1;
    }
    
    /* advance asteroids and update their rotation */
    for (int i = 0; i < MAX_ASTEROIDS; i++){
    	if (asteroids[i].active == 1){
            asteroids[i].x = asteroids[i].x + (asteroids[i].dx);
            asteroids[i].y = asteroids[i].y + (asteroids[i].dy);
            
            //asteroids[i].dx = (
            asteroids[i].phi = asteroids[i].phi + asteroids[i].dphi;
            
            if(asteroids[i].x < 0){
                asteroids[i].x = xMax;
            }
            else if (asteroids[i].x > xMax){
                asteroids[i].x = 0;
            }
            else if(asteroids[i].y < 0){
                asteroids[i].y = yMax;
            }
            else if(asteroids[i].y > yMax){
                asteroids[i].y = 0;
            }
        }
    }
    
    glutPostRedisplay();
    
    // If the player clicks on the start box the game begins.
    if(gameState == 0){
        glutTimerFunc(33, menuMyTimer, value);		/* 30 frames per second */
    }else{
        // Reset the lives at the start of each game.
        lives = 3;
        glutDisplayFunc(myLevelDisplay);
        glutTimerFunc(33, levelMyTimer, 0);
        
    }
}

/* The game timer call back updates the position and velocity of the shit based on the players
 * controls. It checks for collisions between the photon, ship, and asteroids. It also updates the 
 * positions of all asteroids.
 */
void
gameMyTimer(int value)
{
    /*
     *	timer callback function
     */
    
    // Check if the explosion is still happening or to update the ships attributes.
    if(shipExplosion.active == 1){
        shipExplosion.dustTimer = shipExplosion.dustTimer + 1;
    }else{
        /*
         * Update the ships velocity.
         */
        if(left == 1){
            ship.phi = ship.phi + 10;
        }
        if(right == 1){
            ship.phi = ship.phi - 10;
        }
        if(up == 1) {
            ship.engine = 1;
            updateVelocity(0);
        }else if(down == 1) {
            ship.engine = 1;
            updateVelocity(1);
        }else{
            ship.engine = 0;
        }
        
        /* advance the ship */
        if(ship.x < 0){
            ship.x = xMax;
        }
        else if (ship.x > xMax){
            ship.x = 0;
        }
        else if(ship.y < 0){
            ship.y = yMax;
        }
        else if(ship.y > yMax){
            ship.y = 0;
        }
        ship.x = ship.x + ship.dx;
        ship.y = ship.y + ship.dy;
    }
    
    
    /* Update the dust for each frame, its flicker and length.*/
     for (int i = 0; i < MAX_DUST; i++){
         if(dust[i].active){
             if(dust[i].drawThisFrame){
                 dust[i].drawThisFrame = 0;
                 dust[i].dustTimer = dust[i].dustTimer + 1;
             }else{
                 dust[i].drawThisFrame = 1;
                 dust[i].dustTimer = dust[i].dustTimer + 1;
             }
             if(dust[i].dustTimer > 6){
                 dust[i].active = 0;
             }
         }
     }
     
    /* advance photon laser shots, eliminating those that have gone past
     the window boundaries */
    for (int i = 0; i < MAX_PHOTONS; i++){
    	if (photons[i].active == 1){
            photons[i].x = photons[i].x + (photons[i].dx);
            photons[i].y = photons[i].y + (photons[i].dy);
            if(photons[i].x > xMax || photons[i].x < 0 || photons[i].y < 0 || photons[i].y > yMax){
                photons[i].active = 0;
            }
        }
    }
    
    /* advance asteroids and update their rotation */
    for (int i = 0; i < MAX_ASTEROIDS; i++){
    	if (asteroids[i].active == 1){
            asteroids[i].x = asteroids[i].x + (asteroids[i].dx);
            asteroids[i].y = asteroids[i].y + (asteroids[i].dy);
            
            //asteroids[i].dx = (
            asteroids[i].phi = asteroids[i].phi + asteroids[i].dphi;
            
            if(asteroids[i].x < 0){ 
                asteroids[i].x = xMax;
            }
            else if (asteroids[i].x > xMax){
                asteroids[i].x = 0;
            }
            else if(asteroids[i].y < 0){
                asteroids[i].y = yMax;
            }
            else if(asteroids[i].y > yMax){
                asteroids[i].y = 0;
            }
        }
    }

    
    /* test for and handle collisions */
    // Collision between a photon and an asteroid.
    for(int i = 0; i < MAX_PHOTONS; i++){
        if(photons[i].active == 1){
            for(int j = 0; j < MAX_ASTEROIDS; j++){
                if(asteroids[j].active == 1){
                    if(PhotonCollision(&photons[i], &asteroids[j])){
                        activateDust(asteroids[j].x, asteroids[j].y);
                        // Deactivate for the photon that hit and the main asteroid
                        photons[i].active = 0;
                        asteroids[j].active = 0;
                        // Reduce the size of the asteroid based on the size it is now.
                        if(asteroids[j].size == LARGE_SIZE){
                            initAsteroid(&asteroids[findInactiveAsteroid(&asteroids)], asteroids[j].x, asteroids[j].y, MEDIUM_SIZE);
                            initAsteroid(&asteroids[findInactiveAsteroid(&asteroids)], asteroids[j].x, asteroids[j].y, MEDIUM_SIZE);
                        }else if(asteroids[j].size == MEDIUM_SIZE){
                            initAsteroid(&asteroids[findInactiveAsteroid(&asteroids)], asteroids[j].x, asteroids[j].y, SMALL_SIZE);
                            initAsteroid(&asteroids[findInactiveAsteroid(&asteroids)], asteroids[j].x, asteroids[j].y, SMALL_SIZE);
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // Collision between the ship and an asteroid.
    for(int j = 0; j < SHIP_VERTICES; j++){
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            if(asteroids[i].active == 1 && shipExplosion.active == 0){
                if(ShipCollision(&ship.coords[j], &asteroids[i])){
                    activateExplosion(0, 0);
                    lives = lives - 1;
                    j = SHIP_VERTICES + 1;
                    i = MAX_ASTEROIDS + 1;
                }
            }
        }
    }
    
    glutPostRedisplay();
    
    // Checks to see which call back functions to continue on with. Depends on the state of the game.
    if (shipExplosion.dustTimer > TIME_WAIT){
        shipExplosion.active = 0;
        shipExplosion.dustTimer = 0;
        // If there are no lives left load the game over screen.
        if( lives == 0){
            glutDisplayFunc(gameOverDisplay);
            glutTimerFunc(33, gameOverMyTimer, value);
        } else {
            glutDisplayFunc(myLevelDisplay);
            glutTimerFunc(33, levelMyTimer, value);
        }
    }else if(levelBeat()){
        glutTimerFunc(33, gameMyTimer, value);		/* 30 frames per second */
    }else{
        gameState = gameState + 1;
        glutDisplayFunc(myLevelDisplay);
        glutTimerFunc(33, levelMyTimer, value);
    }
}

void
myKey(unsigned char key, int x, int y)
{
    /*
     *	keyboard callback function; add code here for firing the laser,
     *	starting and/or pausing the game, etc.
     */
    switch(key)
    {
        case 32:
            for (int i=0; i<MAX_PHOTONS; i++){
                if (photons[i].active == 0){
                    photons[i].active = 1;
                    photons[i].x = ship.x - 5*sin(ship.phi*DEG2RAD);
                    photons[i].y = ship.y + 5*cos(ship.phi*DEG2RAD);
                    photons[i].dx = -5*sin(ship.phi*DEG2RAD);
                    photons[i].dy = 5*cos(ship.phi*DEG2RAD);
                    break;
                }
            }
            break;
    }
}

/* The mouse click call back function is used to determine if the player clicks
 * on the start button to begin the game in the menu screen.
 */
void
mouseClick(int button, int state, int x, int y){
    if(state == GLUT_DOWN){
        if(gameState == 0){
            if(withinBox(x, y, &startbox)){
                gameState = 1;
            }
        }
    }
}

void
keyPress(int key, int x, int y)
{
    /*
     *	this function is called when a special key is pressed; we are
     *	interested in the cursor keys only
     */
    
    switch (key)
    {
        case 100:
            left = 1; break;
        case 101:
            up = 1; break;
        case 102:
            right = 1; break;
        case 103:
            down = 1; break;
    }
}

void
keyRelease(int key, int x, int y)
{
    /*
     *	this function is called when a special key is released; we are
     *	interested in the cursor keys only
     */
    
    switch (key)
    {
        case 100:
            left = 0; break;
        case 101:
            up = 0; break;
        case 102:
            right = 0; break;
        case 103:
            down = 0; break;
    }
}

void
myReshape(int w, int h)
{
    /*
     *	reshape callback function; the upper and lower boundaries of the
     *	window are at 100.0 and 0.0, respectively; the aspect ratio is
     *  determined by the aspect ratio of the viewport
     */
    
    xMax = 100.0*w/h;
    yMax = 100.0;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, xMax, 0.0, yMax, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
}


/* -- other functions ------------------------------------------------------- */

void
menuInit(){
    // Set up the coordinates system of the start button box so we can check for collisions.
    startbox.coords[0].x = 102; startbox.coords[0].y = 48;
    startbox.coords[1].x = 118; startbox.coords[1].y = 48;
    startbox.coords[2].x = 118; startbox.coords[2].y = 54;
    startbox.coords[3].x = 102; startbox.coords[3].y = 54;
        
    
    // Set up the coordinates of the stars. They will be displayed randomly across the screen.
    for(int i = 0; i < MAX_STARS; i++){
        stars[i].x = myRandom(0, 160);
        stars[i].y = myRandom(0, 100);
    }
    
    /*
     * Set up the asteroids to float through the menu screen.
     * Initialize all the asteroids that are necessary for this level of the
     * game. Each asteroid can have two children so that
     */
    for(int i = 0; i < MAX_LARGE_ASTEROIDS; i++){
        if(myRandom(-1, 1) < 0){
            initAsteroid(&asteroids[i], 0, myRandom(0.0, yMax), LARGE_SIZE);
        }
        else{
            initAsteroid(&asteroids[i], myRandom(0, xMax), 0, LARGE_SIZE);
        }
    }
}
void
gameInit(){
    /*
     * set parameters including the numbers of asteroids and photons present,
     * the maximum velocity of the ship, the velocity of the laser shots, the
     * ship's coordinates and velocity, etc.
     */
    
    // Ships dimensions
    double scaleX = 2;
    double scaleY = 3.5;
    
    /*
     * Set the start position of the ship as well as the initial velocity and
     * angle. The angle points the ship towards the top of the screen.
     */
    ship.x = 83, ship.y = 50, ship.dx = 0, ship.dy = 0, ship.phi = 0; ship.engine = 0;
    ship.coords[0].x = cos(DEG2RAD*90);
    ship.coords[0].y = sin(DEG2RAD*90)*scaleY;
    ship.coords[1].x = cos(DEG2RAD*225)*scaleX;
    ship.coords[1].y = sin(DEG2RAD*225)*scaleY;
    ship.coords[2].x = cos(DEG2RAD*315)*scaleX;
    ship.coords[2].y = sin(DEG2RAD*315)*scaleY;
    
    /*
     * Set the velocity of each of the photon shots that could possibly exist
     * by being shop by the ship.
     */
    for (int i = 0; i < MAX_PHOTONS; i++){
        photons[i].dx = 2.0;
        photons[i].dy = 2.0;
    }
    
    /*
     * Set the point size of the photon shots
     */
    glPointSize(4.0);
    
    /*
     * Initialize all the asteroids that are necessary for this level of the
     * game. Each asteroid can have two children so that 
     */
    for(int i = 0; i < gameState; i++){
        if(myRandom(-1, 1) < 0){
            initAsteroid(&asteroids[i], 0, myRandom(0.0, yMax), LARGE_SIZE);
        }
        else{
            initAsteroid(&asteroids[i], myRandom(0, xMax), 0, LARGE_SIZE);
        }
    }
    
}

void
initAsteroid(Asteroid *a, double x, double y, double size)
{
    /*
     *	generate an asteroid at the given position; velocity, rotational
     *	velocity, and shape are generated randomly; size serves as a scale
     *	parameter that allows generating asteroids of different sizes; feel
     *	free to adjust the parameters according to your needs.
     */
    
    double	theta, r;
    int		i;
    
    a->x = x;
    a->y = y;
    a->dx = myRandom(-0.8, 0.8);
    a->dy = myRandom(-0.8, 0.8);
    a->dphi = myRandom(-0.4, 0.4);
    a->size = size;
    
    a->nVertices = 6+rand()%(MAX_VERTICES-6);
    for (i=0; i<a->nVertices; i++)
    {
        theta = 2.0*M_PI*i/a->nVertices;
        r = size*myRandom(2.0, 3.0);
        a->coords[i].x = -r*sin(theta);
        a->coords[i].y = r*cos(theta);
    }
    
    a->active = 1;
}

// Activate an explosion when the photon hits an asteroid.
void
activateDust(double x, double y){
    for(int i = 0; i < MAX_DUST; i++){
        if(dust[i].active == 0){
            dust[i].active = 1;
            dust[i].drawThisFrame = 1;
            for(int j = 0; j < DUST_PARTICLES; j++){
                dust[i].coords[j].x = myRandom(x-7.5, x+7.5);
                dust[i].coords[j].y = myRandom(y-7.5, y+7.5);
            }
            dust[i].dustTimer = 0;
            break;
        }
    }
}

// Activate an explosion when the ship hits an asteroid.
void
activateExplosion(double x, double y){
    shipExplosion.active = 1;
    shipExplosion.drawThisFrame = 1;
    for(int j = 0; j < DUST_PARTICLES; j++){
        shipExplosion.coords[j].x = myRandom(x-7.5, x+7.5);
        shipExplosion.coords[j].y = myRandom(y-7.5, y+7.5);
    }
    shipExplosion.dustTimer = 0;
}

// Used to draw the ship and its engine flames.
void
drawShip(Ship *s){
    // Check if the engine is on.
    if(s->engine == 1){
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_TRIANGLES);
            glVertex2d(s->coords[0].x,-(s->coords[0].y)-1.0);
            glVertex2d(s->coords[1].x+0.3,s->coords[1].y);
            glVertex2d(s->coords[2].x-0.3,s->coords[2].y);
        glEnd();
    }
    
    // Make the ship white.
    glColor3f(1.0, 1.0, 1.0);
    
    // Draw the ship based on the middle position.
    glBegin(GL_TRIANGLES);
        glVertex2d(s->coords[0].x,s->coords[0].y);
        glVertex2d(s->coords[1].x,s->coords[1].y);
        glVertex2d(s->coords[2].x,s->coords[2].y);
    glEnd();
}
 
// Used to draw the photons.
void
drawPhoton(Photon *p)
{
    // Make the shots white.
    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_POINTS);
        glVertex2f( p->x , p->y);
    glEnd();
    
}

// Used to draw the asteroids.
void
drawAsteroid(Asteroid *a){
    // Have the asteroids be filled up.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Make the asteroids white.
    glColor3f(0.6, 0.6, 0.6);
    
    glBegin(GL_POLYGON);
        for(int i = 0; i < a->nVertices; i++){
            glVertex2d(a->coords[i].x, a->coords[i].y);
        }
    glEnd();
    
    // Reset the mode back to line.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0, 0.0, 0.0);
    
    glBegin(GL_POLYGON);
    for(int i = 0; i < a->nVertices; i++){
        glVertex2d(a->coords[i].x, a->coords[i].y);
    }
    glEnd();
}

// Draw sparkly dust that happens when an asteroid is destroyed.
void
drawDust(Dust *dust){
    // Set the size of the dust to 2.0
    glPointSize(3.0);
    
    glBegin(GL_POINTS);
        for(int i = 0; i < DUST_PARTICLES; i++){
            glColor3f(myRandom(0.0, 1.0),myRandom(0.0, 1.0),myRandom(0.0, 1.0));
            glVertex2d(dust->coords[i].x, dust->coords[i].y);
        }
    glEnd();
    
}

// Draw the lives in the top right corner of the screen. They are just ships! :)
void
drawLives(int i){
    drawShip(&ship);
}

// Draw any text supplied to the position x and y on the screen.
void
drawText(char* text, void * font, double x, double y){
    // Make the title gray.
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen (text); i++)
        glutBitmapCharacter (font, text[i]);
}

/* This is used to draw the menu. It is slightly more hardcoded than I wanted
 * but I didn't manage to find a better solution at the moment.
 */
void
drawMenu(void * font){
    char* startGame = "START";
    char* gameTitle = "ASTEROIDS ";
    
    // Make the title gray.
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(50, 50);
    
    // Draw the game title character by character.
    for (int i = 0; i < strlen (gameTitle); i++)
        glutBitmapCharacter (font, gameTitle[i]);
    
    // Change the colour and draw the start button.
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos2f(105, 50);
    for (int i = 0; i < strlen (startGame); i++)
        glutBitmapCharacter (font, startGame[i]);
    
    // Draw the box around the start button.
    glBegin(GL_POLYGON);
        glVertex2d(startbox.coords[0].x, startbox.coords[0].y);
        glVertex2d(startbox.coords[1].x, startbox.coords[1].y);
        glVertex2d(startbox.coords[2].x, startbox.coords[2].y);
        glVertex2d(startbox.coords[3].x, startbox.coords[3].y);
    glEnd();
}

// Used to draw the stars in the background.
void
drawStars(Stars * stars){
    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_POINTS);
        glVertex2f(stars->x, stars->y);
    glEnd();
}

/* Used to draw the ship in the start screen, was supposed to rotate 
 * around but feature was but on hold for other more pressing issues.
 */
void
drawRotatingShip(){
    // Draw the ship flames on the back.
    if (otherFrame > 0){
        glBegin(GL_TRIANGLES);
            glVertex2d(-2, 2);
            glVertex2d(0, 3);
            glVertex2d(0, 1);
        glEnd();
    }
    
    // Draw the ship body.
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
        glVertex2d(0, 0);
        glVertex2d(0, 4);
        glVertex2d(8, 2);
    glEnd();
}

/* This functions detects if a photon has collided with an asteroid by checking if the number of 
 * lines crossed by the projection of the point in the positive x in odd.
 */
int
PhotonCollision(Photon *p, Asteroid *a){
    double xIntersect = 0.0;
    int lines = a->nVertices;
    int number_intersections = 0;
    // Generate the lines of the asteroid
    double px1, py1, ax1, ay1, ax2, ay2;
    px1 = p->x;
    py1 = p->y;
    
    // Run through each line in the polygon to check if it is a candidate and intersected.
    for(int i = 0; i < a->nVertices; i++){
        ax1 = a->x + a->coords[i%(lines)].x;
        ay1 = a->y + a->coords[i%(lines)].y;
        ax2 = a->x + a->coords[(i+1)%(lines)].x;
        ay2 = a->y + a->coords[(i+1)%(lines)].y;
        // Check to see if it is in between the y values
        if( (py1 < ay1 && py1 > ay2) || (py1 > ay1 && py1 < ay2)){
            xIntersect = (((py1 - ay1)/(ay2-ay1))*ax2) + (((ay2 - py1)/(ay2-ay1))*ax1);
            if((xIntersect >= px1) && (((xIntersect < ax1) && (xIntersect > ax2)) || ((xIntersect > ax1) && (xIntersect < ax2)))){
                number_intersections = number_intersections + 1;
            }
        }
    }
    return number_intersections % 2;
}

/* This functions detects if a ship has collided with an asteroid by checking if the number of
 * lines crossed by the projection of the point in the positive x in odd. This is done for each
 * of the three points of the ship by using they calls to this function.
 */
int
ShipCollision(Coords *c, Asteroid *a){
    // Get the number of vertices in the asteroids which will be the number of lines as well.
    int lines = a->nVertices;
    // Used to store the number of intersections, there is a collision if it is odd.
    int number_intersections = 0;
    // Generate the lines of the asteroid
    double px1, py1, ax1, ay1, ax2, ay2;
    
    // Holds onto the value of where the x intersection occurs on a line.
    double x_intersect = 0.0;
    
    px1 = c->x + ship.x;
    py1 = c->y + ship.y;
    // Check this point aginst the asteroid.
    for(int i = 0; i < a->nVertices; i++){
        ax1 = a->x + a->coords[i%(lines)].x;
        ay1 = a->y + a->coords[i%(lines)].y;
        ax2 = a->x + a->coords[(i+1)%(lines)].x;
        ay2 = a->y + a->coords[(i+1)%(lines)].y;
        // Check to see if it is in between the y values
        if( (py1 <= ay1 && py1 >= ay2) || (py1 >= ay1 && py1 <= ay2)){
            x_intersect = (((py1 - ay1)/(ay2-ay1))*ax2) + (((ay2 - py1)/(ay2-ay1))*ax1);
            if((x_intersect >= px1) && (((x_intersect <= ax1) && (x_intersect >= ax2)) || ((x_intersect >= ax1) && (x_intersect <= ax2)))){
                number_intersections = number_intersections + 1;
            }
        }
    }
    return number_intersections % 2;
}


/* -- helper function ------------------------------------------------------- */

// Returns a random number between a minimum and maximum over a uniform distribution.
double
myRandom(double min, double max){
	double	d;
	
	/* return a random number uniformly draw from [min,max] */
	d = min+(max-min)*(rand()%0x7fff)/32767.0;
	
	return d;
}

// Finds an integer position of an inactive asteroid so it can be used for an initialization of a new one.
int
findInactiveAsteroid(){
    for(int i = 0; i < MAX_ASTEROIDS; i++){
        if(asteroids[i].active == 0){
            return i;
        }
    }
    return -1;
}

// Finds an integer position of an inactive photon so it can be used for an initialization of a new one.
int
findInactivePhoton(){
    for(int i = 0; i < MAX_PHOTONS; i++){
        if(photons[i].active == 0){
            return i;
        }
    }
    return -1;
}

// Finds if a point is within a box. Used specifically for the mouse click which returns pixels.
int
withinBox(double x, double y, StartBox *box){
    // Adjust the x and y values for the screen size.
    x = x/6.0;
    y = y/6.0;
    
    if(x >= box->coords[0].x && x <= box->coords[1].x && y >= box->coords[1].y && y <= box->coords[2].y){
        return 1;
    }else{
        return 0;
    }
}

// Check if there are any asteroids left. If no then the level is over so return 0.
int
levelBeat(){
    int numberLeft = 0;
    for(int i = 0; i < MAX_ASTEROIDS; i++){
        if(asteroids[i].active == 1)
            numberLeft = numberLeft + 1;
    }
    
    if( numberLeft > 0){
        return 1;
    } else {
        return 0;
    }
}

// Returns the appropriate level title depending on the current game state.
char *
getLevelNumber(){
    switch (gameState){
        case 1:
            return "LEVEL 1";
        case 2:
            return "LEVEL 2";
        case 3:
            return "LEVEL 3";
        case 4:
            return "LEVEL 4";
        case 5:
            return "LEVEL 5";
        case 6:
            return "LEVEL 6";
        case 7:
            return "LEVEL 7";
        case 8:
            return "LEVEL 8";
    }
    return "ERROR";
}

/*
 * Helper function used to update the velocity.
 */
void
updateVelocity(int state){
    double acceleration;
    
    // Set the acceleration dependent on the key press state.
    if(state){
        acceleration = ACCELERATION_STEP_BACK;
    }else{
        acceleration = ACCELERATION_STEP_FORWARD;
    }
    
    // If the velocity is not maxed accelerate as normal.
    if((pow((ship.dx - acceleration*sin(ship.phi*DEG2RAD)),2) +
        pow((ship.dy + acceleration*cos(ship.phi*DEG2RAD)),2)) < pow(SHIP_VELOCITY_MAX,2)){
        ship.dx = ship.dx - acceleration*sin(ship.phi*DEG2RAD);
        ship.dy = ship.dy + acceleration*cos(ship.phi*DEG2RAD);
    }else{
        // If the ships velocity change remains all positive.
        if((ship.dx-acceleration*sin(ship.phi*DEG2RAD) >= ship.dx && ship.dy+acceleration*cos(ship.phi*DEG2RAD) >= ship.dy)
           || (pow((ship.dx - acceleration*sin(ship.phi*DEG2RAD)),2) + pow((ship.dy + acceleration*cos(ship.phi*DEG2RAD)),2)) >= pow(SHIP_VELOCITY_MAX,2)){
            if(state){
                ship.dx = SHIP_VELOCITY_MAX*sin(ship.phi*DEG2RAD);
                ship.dy = -SHIP_VELOCITY_MAX*cos(ship.phi*DEG2RAD);
            }else{
                ship.dx = -SHIP_VELOCITY_MAX*sin(ship.phi*DEG2RAD);
                ship.dy = SHIP_VELOCITY_MAX*cos(ship.phi*DEG2RAD);
            }
        }
        // If the ships velocity is decreasing only in the y direction.
        else if(ship.dx-acceleration*sin(ship.phi*DEG2RAD) >= ship.dx && ship.dy+acceleration*cos(ship.phi*DEG2RAD) < ship.dy){
            ship.dx = -SHIP_VELOCITY_MAX*sin(ship.phi*DEG2RAD);
            ship.dy = ship.dy + acceleration*cos(ship.phi*DEG2RAD);
        }
        // If the ships velocity is decreasing only in the x direction.
        else if(ship.dx-acceleration*sin(ship.phi*DEG2RAD) < ship.dx && ship.dy+acceleration*cos(ship.phi*DEG2RAD) >= ship.dy){
            ship.dx = ship.dx - acceleration*sin(ship.phi*DEG2RAD);
            ship.dy = SHIP_VELOCITY_MAX*cos(ship.phi*DEG2RAD);
        }
        // If the ships veloctoiy is decreasing both in x and y direction.
        else{
            ship.dx = ship.dx - acceleration*sin(ship.phi*DEG2RAD);
            ship.dy = ship.dy + acceleration*cos(ship.phi*DEG2RAD);
        }
    }
}
