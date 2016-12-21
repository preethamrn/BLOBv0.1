/**
* NAME: PREETHAM REDDY N.
* PROJECT TITLE: BLOBv0.1
**/

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <fstream>  //FOR INPUTTING AND CHANGING SETTINGS
#include <string>   //STRINGS FOR DOING FILE IO FOR LOADING IMAGE
#include <sstream>  //CONVERTION OF INT TO STRING IN POINTSDISPLAY FUNCTION
#include <time.h>   //FOR RANDOMIZATION OF POWERUP LOCATION
#include <stdlib.h> //SAME AS ABOVE ^^^^^^

using namespace std;

//ALL CONTANTS AND GLOBAL VARIABLES
const int SCREEN_WIDTH = 1336;
const int SCREEN_HEIGHT = 768;
const int SCREEN_BPP = 32;
const int FRAMES_PER_SECOND = 30;
const int PIXELS_PER_TILE = 20;
const int SUPER_SAIYAN_TIME = 5;
const int SUPER_SAIYAN_PROB = 5;
SDL_Surface *screen = NULL;
SDL_Surface *pallette = NULL;
SDL_Event event;
SDL_Rect player, block, camo, bomb, points_bar, win, super, flag;
Mix_Chunk *bombChunk, *trapChunk, *camoChunk, *winChunk, *pointChunk;
Mix_Music *backgroundMusic, *inGameMusic;
SDL_Color textColor = {255,255,255};
TTF_Font *fontlarge = NULL;
TTF_Font *fontsmall = NULL;
Uint32 startTick, endTick, frameTicks;
bool quit, music;

//FUNCTION TO LOAD IMAGE TO A SURFACE GIVEN A FILE NAME
SDL_Surface *load_image( std::string filename ) {
    SDL_Surface* loadedImage = NULL;
    loadedImage = IMG_Load( filename.c_str() );
    return loadedImage;
}
//LOADS FONTS FOR FUTURE USE
bool load_font() {
    fontlarge = TTF_OpenFont("BankGthd_0.ttf", 56);
    fontsmall = TTF_OpenFont("BankGthd_0.ttf", 20);
    if( fontlarge == NULL || fontsmall == NULL )
        return false;
    return true;
}
//APPLY ANY IMAGE TO THE SCREEN GIVEN THE X,Y COORDINATES AND A RECTANGLE SPECIFYING ITS DIMENSIONS
void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL ) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface( source, clip, destination, &offset );
}
//INITIALIZES ALL THE GRAPHICS/AUDIO/FONT COMPONENTS
bool init() {
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
        return false;
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_FULLSCREEN );
    if( screen == NULL )
        return false;
    if( TTF_Init() == -1 )
        return false;
    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
        return false;
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if( initted & flags != flags)
        return false;
    SDL_WM_SetCaption( "BLOBv0.1", NULL );
    return true;
}
//UNINITIALIZES ALL THE GLOBAL VARIABLES WHEN PROGRAM ENDS
void clean_up() {
    SDL_FreeSurface(pallette);
    Mix_FreeChunk(bombChunk);
    Mix_FreeChunk(camoChunk);
    Mix_FreeChunk(trapChunk);
    Mix_FreeChunk(winChunk);
    Mix_FreeChunk(pointChunk);
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeMusic(inGameMusic);
    Mix_CloseAudio();
    TTF_CloseFont(fontlarge);
    TTF_CloseFont(fontsmall);
    TTF_Quit();
    SDL_Quit();
}
//CLEARS THE SPECIFIED PORTION OF THE SCREEN
void clearScreen(int x1 = 0, int y1 = 0, int x2 = SCREEN_WIDTH, int y2 = SCREEN_HEIGHT) {
    player.y=0;
	for(int i=x1; i<x2; i+=PIXELS_PER_TILE)
        for(int j=y1; j<y2; j+=PIXELS_PER_TILE)
            apply_surface(i,j,pallette,screen,&player);
}
//FRAME RATE REGULATION
void frameRegulate(int frameTicks) {
    if( frameTicks < 1000 / FRAMES_PER_SECOND )
        SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - frameTicks );
}
//RETURNS ABSOLUTE VALUE OF X
int abs(int x) {
    if(x >= 0) return x;
    else return -x;
}
//SETS ALL THE RECTANGLES TO BE USED IN DISPLAYING IMAGE
void set_clips() {
    player.x=0; player.y=0; player.w=PIXELS_PER_TILE; player.h=PIXELS_PER_TILE;
	block.x=PIXELS_PER_TILE; block.y=0; block.w=PIXELS_PER_TILE; block.h=PIXELS_PER_TILE;
	camo.x=2*(PIXELS_PER_TILE); camo.y=0; camo.w=PIXELS_PER_TILE; camo.h=PIXELS_PER_TILE;
    bomb.x=4*(PIXELS_PER_TILE); bomb.y=0; bomb.w=PIXELS_PER_TILE; bomb.h=PIXELS_PER_TILE;
	points_bar.x=3*(PIXELS_PER_TILE); points_bar.y=0; points_bar.w=PIXELS_PER_TILE; points_bar.h=PIXELS_PER_TILE;
    win.x=5*(PIXELS_PER_TILE); win.y=0; win.w=3*PIXELS_PER_TILE; win.h=PIXELS_PER_TILE;
    super.x=8*(PIXELS_PER_TILE); super.y=0; super.w=PIXELS_PER_TILE; super.h=PIXELS_PER_TILE;
    flag.x=9*(PIXELS_PER_TILE); flag.y=0; flag.w=PIXELS_PER_TILE; flag.h=PIXELS_PER_TILE;
}
//CONSTANTS
enum { MAX_PLAYERS=4, MAX_BOMBS=100, MAX_BOARD_WIDTH=(int)(SCREEN_WIDTH/PIXELS_PER_TILE), MAX_BOARD_HEIGHT=(int)(SCREEN_HEIGHT/PIXELS_PER_TILE) };
enum { WHITE = 0, RED = 1, BLUE = 2, GREEN = 3, YELLOW = 4, ZOMBIE = 5};
//BOARDS THAT STORE DATA ABOUT GAME STATE
char BOARD[MAX_BOARD_WIDTH][MAX_BOARD_HEIGHT];
int COLOR_MATRIX[MAX_BOARD_WIDTH][MAX_BOARD_HEIGHT];
int HASHMAP[2][MAX_BOARD_WIDTH][MAX_BOARD_HEIGHT];
int PICKUP_X, PICKUP_Y;

int WIN_AMOUNT, PLAYERS, BOARD_WIDTH, BOARD_HEIGHT, ZOMBIE_AMOUNT;


class BOMB {
	public:
	int X, Y, BOMB_timer, playerNo, BOMB_type;
	BOMB();
	BOMB(int mBOMB_timer, int mplayerNo, int mBOMB_type, int mX, int mY);
	void counter();
	void explode();
	void destroy();
};

BOMB::BOMB() {
	BOMB_timer=0; BOMB_type=0; X=-1; Y=-1; playerNo=0;
}

BOMB::BOMB(int mBOMB_timer, int mplayerNo, int mBOMB_type, int mX, int mY) {
	BOMB_timer=mBOMB_timer;
	BOMB_type=mBOMB_type;
	X=mX;
	Y=mY;
	playerNo=mplayerNo;
}
//REMOVES BOMB FROM ARRAY
void BOMB::destroy() {
	for(int i=0; i<BOARD_WIDTH; i++) {
		HASHMAP[BOMB_type][i][Y] = WHITE;
	}
	for(int i=0; i<BOARD_HEIGHT; i++) {
		HASHMAP[BOMB_type][X][i] = WHITE;
	}
}
//COUNTER TO CHECK WHETHER BOMB SHOULD BE DESTROYED OR NOT
void BOMB::counter() {
	if(BOMB_timer) {
		BOMB_timer--;
		if(BOMB_timer==0) {
			destroy();
		}
	}
}
//PUTS BOMB ONTO ARRAY
void BOMB::explode() {
	for(int i=0; i<BOARD_WIDTH; i++) {
		HASHMAP[BOMB_type][i][Y] = playerNo;
	}
	for(int i=0; i<BOARD_HEIGHT; i++) {
		HASHMAP[BOMB_type][X][i] = playerNo;
	}
}
//BASE CLASS FOR ANY OBJECT THAT CAN MOVE ON SCREEN (ie: ZOMBIES AND PLAYERS)
class MOVABLE {
public:
	char CONTROLS[7];
    int X_POS, Y_POS, playerNo, X, Y;
    MOVABLE();
	int input(char InputValue);
	char GOTO(int x, int y);
	void counter();
};

MOVABLE::MOVABLE() {
    X=0; Y=0; playerNo=0; X_POS=0; Y_POS=0;
}
//SETS THE VELOCITIES OF PLAYERS GIVEN THE INPUT
int MOVABLE::input(char InputValue) {
	if(InputValue==CONTROLS[0]) {
		Y=-1; X=0;
	} else if(InputValue==CONTROLS[1]) {
		Y=+1; X=0;
	} else if(InputValue==CONTROLS[2]) {
		X=-1; Y=0;
	} else if(InputValue==CONTROLS[3]) {
		X=+1; Y=0;
	} else {
        return 0;
	}
}
//COUNTER TO MOVE OBJECT ACROSS ARRAY
void MOVABLE::counter() {
    BOARD[X_POS][Y_POS]=' ';
    COLOR_MATRIX[X_POS][Y_POS]=WHITE;
    X_POS+=X;
    Y_POS+=Y;
    if(X_POS < 0) X_POS+=1;
    else if(X_POS > BOARD_WIDTH-1) X_POS-=1;
    else if(Y_POS < 0) Y_POS+=1;
    else if(Y_POS > BOARD_HEIGHT-1) Y_POS-=1;
    BOARD[X_POS][Y_POS]='0';
    COLOR_MATRIX[X_POS][Y_POS]=playerNo;
}
//RETURNS THE OUTPUT FOR OBJECT TO GET TO THE LOCATION AT X,Y
char MOVABLE::GOTO(int x, int y) {
    char OutputValue;
    if(abs(y - Y_POS) > abs(x - X_POS)) {
        if(y < Y_POS) OutputValue = CONTROLS[0];
        else if(y > Y_POS) OutputValue = CONTROLS[1];
    } else {
        if(x < X_POS) OutputValue = CONTROLS[2];
        else if(x > X_POS) OutputValue = CONTROLS[3];
    }
    return OutputValue;
}

class GUN: public MOVABLE {
public:
	int POINTS, BOMB_no, init_X, init_Y, superSaiyan, FLAG_X, FLAG_Y, FLAG_picked;
	BOMB bomb[MAX_BOMBS];
	GUN();
	GUN(int minit_X, int minit_Y, int mplayerNo);
	int input(char InputValue);
	char autopilot();
	void counter();
};
GUN *Player = new GUN[MAX_PLAYERS];

GUN::GUN() {
    X=0; Y=0; POINTS=0; BOMB_no=20, superSaiyan=0; X_POS = 0; Y_POS = 0; FLAG_X=0; FLAG_Y=0; FLAG_picked=0;
}

GUN::GUN(int minit_X, int minit_Y, int mplayerNo) {
	X=0;
	Y=0;
	POINTS=0;
	BOMB_no=20;
	superSaiyan=0;
	init_X = minit_X;
	init_Y = minit_Y;
	X_POS = init_X;
	Y_POS = init_Y;
	playerNo = mplayerNo;
	FLAG_X=init_X; FLAG_Y=init_Y; FLAG_picked=0;
}
//FINDS AND DOES THE BEST OPTION FOR A PLAYER TO OPTIMIZE POINTS
char GUN::autopilot() {
    char OutputValue=0;
    //IF THE CURRENT LOCATION ISN'T SAFE (NOT WHITE OR SAME COLOR AS GUN)
    if(HASHMAP[0][X_POS][Y_POS]!=WHITE && HASHMAP[0][X_POS][Y_POS]!=playerNo) {
        //FIND WHETHER TO MOVE UP, DOWN, LEFT OR RIGHT TO AVOID FURTHER DAMAGE
        if(X_POS < BOARD_WIDTH-1) {
            if(HASHMAP[0][X_POS+1][Y_POS]!=WHITE && HASHMAP[0][X_POS+1][Y_POS]!=playerNo) {
                if(Y_POS > BOARD_HEIGHT/2) OutputValue = CONTROLS[0];
                else OutputValue = CONTROLS[1];
            }
        } else if(X_POS == BOARD_WIDTH-1) {
            if(HASHMAP[0][X_POS-1][Y_POS]!=WHITE && HASHMAP[0][X_POS-1][Y_POS]!=playerNo) {
                if(Y_POS > BOARD_HEIGHT/2)OutputValue = CONTROLS[0];
                else OutputValue = CONTROLS[1];
            }
        } else if(Y_POS < BOARD_HEIGHT-1) {
            if(HASHMAP[0][X_POS][Y_POS+1]!=WHITE && HASHMAP[0][X_POS][Y_POS+1]!=playerNo) {
                if(X_POS > BOARD_WIDTH/2) OutputValue = CONTROLS[2];
                else OutputValue = CONTROLS[3];
            }
        } else if(Y_POS == BOARD_HEIGHT-1) {
            if(HASHMAP[0][X_POS][Y_POS-1]!=WHITE && HASHMAP[0][X_POS][Y_POS-1]!=playerNo) {
                if(X_POS > BOARD_WIDTH/2) OutputValue = CONTROLS[2];
                else OutputValue = CONTROLS[3];
            }
        }
        if(!OutputValue) OutputValue = GOTO(PICKUP_X, PICKUP_Y);
        return OutputValue;
    }
    //IF GUN STILL HAS BOMBS
    if(BOMB_no) {
        //EXPLODE BOMB IF ANY PLAYERS ARE IN SAME ROW OR COLUMN
        for(int i=0; i<PLAYERS; i++) {
            if(i!= playerNo-1 && (Player[i].X_POS == X_POS || Player[i].Y_POS == Y_POS)) {
                OutputValue = CONTROLS[5];
                return OutputValue;
            }
        }
    } else {
        //IF NO BOMBS, GO TOWARDS A BOMB
        OutputValue = GOTO(PICKUP_X, PICKUP_Y);
        return OutputValue;
    }
    //RANDOMLY CHOOSE TO GO TOWARDS BOMB OR NEAREST PLAYER
    if(BOMB_no < MAX_BOMBS - 2 && !rand()%4) {
        OutputValue = GOTO(PICKUP_X, PICKUP_Y);
    } else {
        for(int i=0; i<PLAYERS; i++) {
            if(playerNo > i+1) {
                OutputValue = GOTO(Player[i].X_POS, Player[i].Y_POS);
            } else if(playerNo < i+1) {
                if(Player[i].X_POS == X_POS) {
                    if(X_POS > BOARD_WIDTH/2)
                        OutputValue = CONTROLS[2];
                    else if(X_POS >= BOARD_WIDTH/2)
                        OutputValue = CONTROLS[3];
                } else if(Player[i].Y_POS == Y_POS) {
                    if(Y_POS > BOARD_HEIGHT/2)
                        OutputValue = CONTROLS[0];
                    else if(Y_POS >= BOARD_HEIGHT/2)
                        OutputValue = CONTROLS[1];
                }
            }
        }
    }
    return OutputValue;
}
//BASED ON THE GIVEN INPUT, DO THE REQUIRED OUTPUT
int GUN::input(char InputValue) {
	int returnValue = MOVABLE::input(InputValue);
	if(returnValue) {
        return returnValue;
	} else if(InputValue==CONTROLS[4]) {
		X=0; Y=0;
		if(BOMB_no) {
            if(!superSaiyan)
                BOMB_no--;
			bomb[BOMB_no].X=X_POS;
			bomb[BOMB_no].Y=Y_POS;
			bomb[BOMB_no].BOMB_timer=FRAMES_PER_SECOND * (1.5);
			bomb[BOMB_no].BOMB_type=1;
			bomb[BOMB_no].playerNo=playerNo;
			bomb[BOMB_no].explode();
			if(music)
                Mix_PlayChannel(-1, camoChunk, 0);
		}
	} else if(InputValue==CONTROLS[5]) {
		if(BOMB_no) {
			if(!superSaiyan)
                BOMB_no--;
			bomb[BOMB_no].destroy();
			bomb[BOMB_no].X=X_POS;
			bomb[BOMB_no].Y=Y_POS;
			bomb[BOMB_no].BOMB_timer=FRAMES_PER_SECOND * (0.5);
			bomb[BOMB_no].BOMB_type=0;
			bomb[BOMB_no].playerNo=playerNo;
			bomb[BOMB_no].explode();
            if(music)
                Mix_PlayChannel(-1, bombChunk, 0);
		}
	} else if(InputValue==CONTROLS[6]) {
		if(BOMB_no) {
			if(!superSaiyan)
                BOMB_no--;
			bomb[BOMB_no].X=X_POS;
			bomb[BOMB_no].Y=Y_POS;
			bomb[BOMB_no].BOMB_timer=0;
			bomb[BOMB_no].playerNo=playerNo;
			HASHMAP[0][bomb[BOMB_no].X][bomb[BOMB_no].Y]=playerNo;
			if(music)
                Mix_PlayChannel(-1, trapChunk, 0);
		}
	} else return 0;
	return 1;
}
//PERFORM ALL ACTIONS THAT NEED TO BE DONE AFTER MOVING
void GUN::counter() {
    MOVABLE::counter();
    if(superSaiyan) superSaiyan--;
    for(int i=0; i<MAX_BOMBS; i++) bomb[i].counter();
}

class ZOMBIES: public MOVABLE {
public:
    int restTime, INITrestTime, zombieNo;
    ZOMBIES();
    void respawn();
    void counter();
};
ZOMBIES *Zombie;

ZOMBIES::ZOMBIES() {
    INITrestTime = 0.1 * FRAMES_PER_SECOND;
    restTime = INITrestTime;
    playerNo = ZOMBIE;
    zombieNo = 0;
    CONTROLS[0] = '0'; CONTROLS[1] = '1'; CONTROLS[2] = '2';  CONTROLS[3] = '3';
    X_POS = 0;
	Y_POS = 0;
}
//RESPAWN ZOMBIE IF DEAD
void ZOMBIES::respawn() {
    BOARD[X_POS][Y_POS]=' ';
    COLOR_MATRIX[X_POS][Y_POS]=WHITE;
    do {
        X_POS = (rand())%BOARD_WIDTH;
        Y_POS = (rand())%BOARD_HEIGHT;
    } while(abs(X_POS - Player[0].X_POS) < 4 || abs(Y_POS - Player[0].Y_POS) < 4);
}
//PERFORM ALL ACTIONS TO BE DONE BETWEEN MOVES
void ZOMBIES::counter() {
    //IF NOT RESTING
    if(!restTime) {
        //DEPENDING ON ZOMBIE NUMBER, EITHER GUARD A BOMB OR CHASE PLAYER
        if(zombieNo%5 == 0)
            input(GOTO(PICKUP_X + (1 - 2*(rand()%2)) * rand()%4, PICKUP_Y + (1 - 2*(rand()%2)) * rand()%4));
        else
            input(GOTO(Player[0].X_POS, Player[0].Y_POS));
        restTime = INITrestTime;
        MOVABLE::counter();
    } else {
        restTime--;
    }
}
//ADD MORE ZOMBIES TO THE ARRAY USING POINTERS
ZOMBIES *resizeArray(ZOMBIES *m, int oldsize, int newsize) {
    ZOMBIES *p = new ZOMBIES[newsize];
    for(int i=0; i<oldsize; i++) p[i] = m[i];
    delete m;
    return p;
}

class GAME {
    void initialize();
    void poweruper();
    void display_screen();
    int pointsDisplay();
    int collision_check();
    int zombie_collision_check();
    int capture_collision_check();
public:
    int reactionTime, INITreactionTime;
    int gameType;
    GAME();
    int play();
};
//GETS ALL DATA FOR GAME FROM FILE AND STARTS IT
GAME::GAME() {
    ifstream settings_input("BLOBsettings.txt");
    settings_input >> PLAYERS >> BOARD_WIDTH >> BOARD_HEIGHT >> WIN_AMOUNT >> ZOMBIE_AMOUNT;
    for(int i=0; i<PLAYERS; i++) {
        settings_input >> Player[i].CONTROLS[0] >> Player[i].CONTROLS[1] >> Player[i].CONTROLS[2] >> Player[i].CONTROLS[3] >> Player[i].CONTROLS[4] >> Player[i].CONTROLS[5] >> Player[i].CONTROLS[6];
        Player[i].X=0; Player[i].Y=0; Player[i].POINTS=0; Player[i].BOMB_no=20, Player[i].superSaiyan=0; Player[i].X_POS = Player[i].init_X; Player[i].Y_POS = Player[i].init_Y;
    }
    INITreactionTime = 0.2 * FRAMES_PER_SECOND;
    reactionTime = INITreactionTime;
}
//PERFORMS ALL NECESSARY FUNCTIONS BEFORE GAME STARTS
void GAME::initialize() {
    if(music) {
        Mix_HaltMusic();
        Mix_PlayMusic(inGameMusic, -1);
    }
    //RESETS THE BOARDS
	for(int i=0; i<BOARD_HEIGHT; i++) {
		for(int j=0; j<BOARD_WIDTH; j++) {
			BOARD[j][i]=' ';
			COLOR_MATRIX[j][i]=WHITE;
			HASHMAP[0][j][i]=WHITE;
			HASHMAP[1][j][i]=WHITE;
		}
	}
	PICKUP_X=BOARD_WIDTH/2;
	PICKUP_Y=BOARD_HEIGHT/2;

	srand(time(NULL));
	switch(PLAYERS) {
		//ADD MORE PLAYERS HERE-------------------------------------
        case 4: Player[3].init_X=BOARD_WIDTH-1;
				Player[3].init_Y=0;
				Player[3].X_POS=Player[3].init_X;
				Player[3].Y_POS=Player[3].init_Y;
				Player[3].FLAG_X=Player[3].init_X;
				Player[3].FLAG_Y=Player[3].init_Y;
				Player[3].playerNo=YELLOW;

		case 3: Player[2].init_X=0;
				Player[2].init_Y=BOARD_HEIGHT-1;
				Player[2].X_POS=Player[2].init_X;
				Player[2].Y_POS=Player[2].init_Y;
				Player[2].FLAG_X=Player[2].init_X;
				Player[2].FLAG_Y=Player[2].init_Y;
				Player[2].playerNo=GREEN;

		case 2: Player[1].init_X=BOARD_WIDTH-1;
				Player[1].init_Y=BOARD_HEIGHT-1;
				Player[1].X_POS=Player[1].init_X;
				Player[1].Y_POS=Player[1].init_Y;
				Player[1].FLAG_X=Player[1].init_X;
				Player[1].FLAG_Y=Player[1].init_Y;
				Player[1].playerNo=BLUE;

		case 1: Player[0].init_X=0;
				Player[0].init_Y=0;
				Player[0].X_POS=Player[0].init_X;
				Player[0].Y_POS=Player[0].init_Y;
				Player[0].FLAG_X=Player[0].init_X;
				Player[0].FLAG_Y=Player[0].init_Y;
				Player[0].playerNo=RED;
	}
	//SETS UP BOMBS
	for(int i=0; i<PLAYERS; i++)
		for(int j=0; j<MAX_BOMBS; j++)
			Player[i].bomb[j].playerNo=Player[i].playerNo;
    //SET UP SCREEN
	clearScreen();
    block.y=0;
	for(int i=0; i<BOARD_HEIGHT+2; i++) {
		for(int j=0; j<BOARD_WIDTH+2; j++) {
			if(i==0 || i==BOARD_HEIGHT+1 || j==0 || j==BOARD_WIDTH+1) apply_surface(PIXELS_PER_TILE*i, PIXELS_PER_TILE*j, pallette, screen, &block);
		}
	}
    //IF ZOMBIES MODE, CREATE ZOMBIES
    if(gameType == 3) {
        PLAYERS = 1;
        Zombie = new ZOMBIES[ZOMBIE_AMOUNT];
        for(int i=0; i<ZOMBIE_AMOUNT; i++) {
            Zombie[i].zombieNo = i;
            Zombie[i].respawn();
        }
    }
}

void GAME::display_screen() {
    //SHOWS ALL THE TILES OF THE BOARD
	for(int i=0; i<BOARD_HEIGHT; i++) {
		for(int j=0; j<BOARD_WIDTH; j++) {
            //SHOW THE CAMOFLAGE
            if(HASHMAP[1][j][i]!=WHITE) {
                camo.y = PIXELS_PER_TILE*(HASHMAP[1][j][i]);
                apply_surface(PIXELS_PER_TILE*(j+1),PIXELS_PER_TILE*(i+1),pallette,screen,&camo);
			}
			//SHOW THE PLAYER
			else if(BOARD[j][i]=='0') {
                int pNo = COLOR_MATRIX[j][i];
                if(pNo <= YELLOW && Player[pNo - 1].superSaiyan) {
                    super.y = PIXELS_PER_TILE*(pNo);
                    apply_surface(PIXELS_PER_TILE*(j+1),PIXELS_PER_TILE*(i+1),pallette,screen,&super);
                } else {
                    player.y = PIXELS_PER_TILE*(pNo);
                    apply_surface(PIXELS_PER_TILE*(j+1),PIXELS_PER_TILE*(i+1),pallette,screen,&player);
                }
            }
            //SHOW THE BOMBS AND TRAPS
            else if(HASHMAP[0][j][i]!=WHITE) {
                block.y = PIXELS_PER_TILE*(HASHMAP[0][j][i]);
                apply_surface(PIXELS_PER_TILE*(j+1),PIXELS_PER_TILE*(i+1),pallette,screen,&block);
			} else {
                player.y = 0;
                apply_surface(PIXELS_PER_TILE*(j+1),PIXELS_PER_TILE*(i+1),pallette,screen,&player);
            }
		}
	}
    //SHOWS SIDES OF BOARD
    block.y=0;
	for(int i=0; i<BOARD_HEIGHT+2; i++) {
		for(int j=0; j<BOARD_WIDTH+2; j++) {
			if(i==0 || i==BOARD_HEIGHT+1 || j==0 || j==BOARD_WIDTH+1) apply_surface(PIXELS_PER_TILE*j, PIXELS_PER_TILE*i, pallette, screen, &block);
		}
	}
	//DISPLAYS POWERUP
	camo.y = 0;
	apply_surface(PIXELS_PER_TILE*(PICKUP_X+1),PIXELS_PER_TILE*(PICKUP_Y+1),pallette,screen,&camo);
    //DISPLAYS EXIT
	win.y=0;
	apply_surface(SCREEN_WIDTH-win.w,SCREEN_HEIGHT-win.h,pallette,screen,&win);

    int xStart = (BOARD_WIDTH + 3) * PIXELS_PER_TILE;
    int bombsPerRow = (SCREEN_WIDTH - xStart)/bomb.w;
    int bombHeightPerPlayer = bomb.h*((MAX_BOMBS / bombsPerRow) + 2);
    for(int i=0; i<PLAYERS; i++) {
        //DISPLAYS BOMBS OF EACH PLAYER
        bomb.y = (i+1)*PIXELS_PER_TILE;
        for(int j=0; j<Player[i].BOMB_no; j++)
            apply_surface(xStart+(j%bombsPerRow)*(bomb.w), bomb.h * (j/bombsPerRow) + bombHeightPerPlayer*i, pallette, screen, &bomb);
        bomb.y=0;
        for(int j=Player[i].BOMB_no; j<MAX_BOMBS; j++)
            apply_surface(xStart+(j%bombsPerRow)*(bomb.w), bomb.h * (j/bombsPerRow) + bombHeightPerPlayer*i, pallette, screen, &bomb);
        //DISPLAYS EACH PLAYER'S FLAG
        if(gameType==2) {
            flag.y = (i+1)*PIXELS_PER_TILE;
            apply_surface((Player[i].FLAG_X+1)*PIXELS_PER_TILE,(Player[i].FLAG_Y+1)*PIXELS_PER_TILE, pallette, screen, &flag);
        }
    }
    //DISPLAYS ALL ZOMBIES
    if(gameType == 3) {
        int zombiesPerRow = (SCREEN_WIDTH - xStart)/player.w;
        player.y = ZOMBIE * PIXELS_PER_TILE;
        for(int j=0; j<ZOMBIE_AMOUNT; j++) {
            apply_surface(xStart+(j%zombiesPerRow)*(player.w), player.h * (j/bombsPerRow) + bombHeightPerPlayer, pallette, screen, &player);
        }
    }
}
//RELOCATES THE POWERUP EACH TIME SOMEONE PICKS IT UP
void GAME::poweruper() {
	PICKUP_X = rand()%BOARD_WIDTH;
	PICKUP_Y = rand()%BOARD_HEIGHT;
}
//FUNCTION TO DISPLAY POINTS AND ALSO RETURNS THE WINNER (IF PRESENT)
int GAME::pointsDisplay() {
    int lengthOfWinTileBar;
    if(WIN_AMOUNT) {
        lengthOfWinTileBar = (BOARD_WIDTH*PIXELS_PER_TILE)/WIN_AMOUNT;
        for(int i=0; i<PLAYERS; i++) {
            if(Player[i].POINTS >= WIN_AMOUNT) {
                win.y=(i+1)*PIXELS_PER_TILE;
                apply_surface(SCREEN_WIDTH-win.w,SCREEN_HEIGHT-win.h,pallette,screen,&win);
                return i;
            } else {
                points_bar.y=0;
                apply_surface((WIN_AMOUNT + 1)*(lengthOfWinTileBar),SCREEN_HEIGHT-PIXELS_PER_TILE*(PLAYERS-i),pallette,screen,&points_bar);
            }
            points_bar.y=(i+1)*PIXELS_PER_TILE;
            apply_surface(Player[i].POINTS*(lengthOfWinTileBar),SCREEN_HEIGHT-PIXELS_PER_TILE*(i+1),pallette,screen,&points_bar);
        }
    } else {
        clearScreen(0, SCREEN_HEIGHT-PLAYERS*PIXELS_PER_TILE, SCREEN_WIDTH-3*PIXELS_PER_TILE, SCREEN_HEIGHT);
        for(int i=0; i<PLAYERS; i++) {
            std::string points_string = "PLAYER ";
            int n = Player[i].POINTS;
            stringstream st; st << (i+1) << ": " << n; points_string += st.str();
            SDL_Surface *message = TTF_RenderText_Solid(fontsmall, points_string.c_str(), textColor);
            apply_surface(PIXELS_PER_TILE, SCREEN_HEIGHT-PIXELS_PER_TILE*(PLAYERS-i), message, screen);
        }
    }
	return -1;
}
//CHECKS FOR COLLISIONS WITH ANY OF THE POSSIBLE OBJECTS ON BOARD IN NORMAL OR AI MODE
int GAME::collision_check() {
	for(int i=0; i<PLAYERS; i++)
		for(int j=0; j<PLAYERS; j++)
            if(HASHMAP[0][Player[i].X_POS][Player[i].Y_POS]==Player[j].playerNo && i!=j) {
                Player[j].POINTS++;
                if(music)
                    Mix_PlayChannel(-1, pointChunk, 0);
            }
    for(int i=0; i<PLAYERS; i++) {
        if(PICKUP_X==Player[i].X_POS && PICKUP_Y==Player[i].Y_POS && Player[i].BOMB_no <= MAX_BOMBS - 2) {
			Player[i].BOMB_no+=2;
			if(!(time(NULL)%SUPER_SAIYAN_PROB)) Player[i].superSaiyan = FRAMES_PER_SECOND * SUPER_SAIYAN_TIME;
			poweruper();
		}
    }
    return pointsDisplay();
}

//CHECKS FOR COLLISIONS WITH ANY OF THE POSSIBLE OBJECTS ON BOARD IN ZOMBIES MODE
int GAME::zombie_collision_check() {
    bool makeNewZombie = false;
    for(int i=0; i<ZOMBIE_AMOUNT; i++) {
        if(Player[0].X_POS == Zombie[i].X_POS && Player[0].Y_POS == Zombie[i].Y_POS) {
            return Zombie[i].playerNo - 1;
        } else if(HASHMAP[0][Zombie[i].X_POS][Zombie[i].Y_POS] != WHITE) {
            Player[0].POINTS++;
            if(music)
                Mix_PlayChannel(-1, pointChunk, 0);
            HASHMAP[0][Zombie[i].X_POS][Zombie[i].Y_POS] = WHITE;
            Zombie[i].respawn();
            makeNewZombie = true;
        }
    }
    if(makeNewZombie && !(rand()%3)) {
        int n = rand()%3;
        Zombie = resizeArray(Zombie, ZOMBIE_AMOUNT, ZOMBIE_AMOUNT+n);
        for(int i=0; i<n; i++) {
            ZOMBIE_AMOUNT++;
            Zombie[ZOMBIE_AMOUNT-1].respawn();
            Zombie[ZOMBIE_AMOUNT-1].zombieNo = ZOMBIE_AMOUNT-1;
        }
    }
    for(int i=0; i<PLAYERS; i++) {
        if(PICKUP_X==Player[i].X_POS && PICKUP_Y==Player[i].Y_POS && Player[i].BOMB_no <= MAX_BOMBS - 2) {
			Player[i].BOMB_no+=2;
			if(!(time(NULL)%SUPER_SAIYAN_PROB)) Player[i].superSaiyan = FRAMES_PER_SECOND * SUPER_SAIYAN_TIME;
			poweruper();
		}
    }
    return pointsDisplay();
}

//CHECKS FOR COLLISIONS WITH ANY OF THE POSSIBLE OBJECTS ON BOARD IN CAPTURE THE FLAG MODE
int GAME::capture_collision_check() {
    //CHECKS FOR COLLISION WITH FLAGS OR ANY OF THE BOMBS
	for(int i=0; i<PLAYERS; i++) {
		for(int j=(i+1)%2; j<PLAYERS; j+=2) {
			if(HASHMAP[0][Player[i].X_POS][Player[i].Y_POS]==Player[j].playerNo && i!=j) {
				Player[j].POINTS++;
                if(music)
                    Mix_PlayChannel(-1, pointChunk, 0);
				if(Player[j].FLAG_picked == i+1) {
					Player[j].FLAG_picked=0;
					Player[j].FLAG_X=Player[i].X_POS; Player[j].FLAG_Y=Player[i].Y_POS;
					Player[i].X_POS=Player[i].init_X; Player[i].Y_POS=Player[i].init_Y;
				} else if(Player[(j+2)%4].FLAG_picked == i+1) {
					Player[(j+2)%4].FLAG_picked=0;
					Player[(j+2)%4].FLAG_X=Player[i].X_POS; Player[(j+2)%4].FLAG_Y=Player[i].Y_POS;
					Player[i].X_POS=Player[i].init_X; Player[i].Y_POS=Player[i].init_Y;
				}
			}
			if(Player[i].FLAG_X==Player[j].X_POS && Player[i].FLAG_Y==Player[j].Y_POS && i!=j) {
				Player[i].FLAG_picked=j+1;
			}
		}
	}
	for(int i=0; i<PLAYERS; i++) {
        //CHECK FOR PICKING UP POWERUP
		if(PICKUP_X==Player[i].X_POS && PICKUP_Y==Player[i].Y_POS) {
			Player[i].BOMB_no+=2;
			poweruper();
		}
		//CHECK IF PLAYER'S FLAG IS PICKED UP
		if(Player[i].FLAG_picked) {
			int j=Player[i].FLAG_picked-1;
			Player[i].FLAG_X=Player[j].X_POS;
			Player[i].FLAG_Y=Player[j].Y_POS;
			if(Player[j].X_POS==Player[j].init_X && Player[j].Y_POS==Player[j].init_Y) {
				Player[j].POINTS+=10;
                if(music)
                    Mix_PlayChannel(-1, pointChunk, 0);
				Player[i].FLAG_X=Player[i].init_X;
				Player[i].FLAG_Y=Player[i].init_Y;
				Player[i].FLAG_picked=0;
			}
		}
	}
	return pointsDisplay();
}
//FUNCTION TO START PLAYING A NEW GAME
int GAME::play() {
    initialize();
    bool quit_game = false;
	while(!quit_game) {
        startTick = SDL_GetTicks();
        while( SDL_PollEvent( &event ) ) {
            if( event.type == SDL_QUIT )
                quit = true;
            //CHECKS IF USER EXITS
            else if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                int x, y;
                SDL_GetMouseState( &x, &y );
                if(x>=SCREEN_WIDTH-3*PIXELS_PER_TILE && y >= SCREEN_HEIGHT-PIXELS_PER_TILE) {
                    quit_game = true;
                    if(music) {
                        Mix_HaltMusic();
                        Mix_PlayMusic(backgroundMusic, -1);
                    }
                }
            }
            //CHECKS IF USER PRESSES A KEY (FOR MOVING OR PLACING BOMBS)
            else if( event.type == SDL_KEYDOWN ) {
                char InputValue = event.key.keysym.sym;
                int i=0;
                while(!Player[i].input(InputValue) && i<PLAYERS) i++;
            }
        }
        //CODE TO ENGAGE AUTOPILOT FOR ALL PLAYERS IF PLAYING VS CPU
        if(gameType == 1) {
            for(int i=1; i<PLAYERS; i++)
                if(!reactionTime || !Player[i].BOMB_no || Player[i].superSaiyan)
                    Player[i].input(Player[i].autopilot());
            if(!reactionTime) reactionTime = INITreactionTime;
            else reactionTime--;
        }
        //CODE TO PLAY ZOMBIES MODE
        else if(gameType == 3) {
            for(int i=0; i<ZOMBIE_AMOUNT; i++) {
                Zombie[i].counter();
            }
        }
        //GOES TO NEXT MOVE OF EACH PLAYER
        for(int i=0; i<PLAYERS; i++)
            Player[i].counter();
        int winplayer = -1;
        //CALLS THE COLLISION CHECK FUNCTIONS FOR SPECIFIC GAME MODE
        if(gameType == 0 || gameType == 1) winplayer = collision_check();
        else if(gameType == 2) winplayer = capture_collision_check();
        else if(gameType == 3) winplayer = zombie_collision_check();
		display_screen();
		if( SDL_Flip( screen ) == -1 )
            return -1;
        //CHECKS IF ANY PLAYER HAS WON
        if(winplayer!=-1) {
            //DISPLAYS THE PLAYER WHO HAS WON
            SDL_Surface *winnerTile = load_image("bigTiles.png");
            SDL_Rect winnerClip; winnerClip.x=0; winnerClip.y=200*(winplayer+1); winnerClip.w=500; winnerClip.h=200;
            apply_surface(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 100, winnerTile, screen, &winnerClip);
            SDL_Flip(screen);
            if(music)
                Mix_PlayChannel(-1, winChunk, 0);
            //DELETES ALL THE ZOMBIES AND QUITS GAME
            delete Zombie;
            while(!quit_game) {
                startTick = SDL_GetTicks();
                while(SDL_PollEvent(&event)) {
                    if(event.type==SDL_MOUSEBUTTONDOWN) quit_game = true;
                }
                endTick = SDL_GetTicks();
                frameTicks = endTick - startTick;
                frameRegulate(frameTicks);
            }
            clearScreen();
            if(music) {
                Mix_HaltMusic();
                Mix_PlayMusic(backgroundMusic, -1);
            }
            return winplayer;
        }
        //FRAME REGULATION
        endTick = SDL_GetTicks();
        frameTicks = endTick - startTick;
        frameRegulate(frameTicks);
	}
	return 0;
}

void settingsDisplay(SDL_Surface *settings_image, SDL_Surface *yellowtile, char CONTROLSarray[][7]) {
    //APPLIES SETTINGS SURFACE
    clearScreen();
    apply_surface(0, 0, settings_image, screen);
    for(int i=0; i<MAX_PLAYERS; i++) {
        for(int j=0; j<7; j++) {
            SDL_Surface *text;
            char control[2] = {CONTROLSarray[i][j],'\0'};
            text = TTF_RenderText_Solid(fontlarge, control, textColor);
            apply_surface(180+90*j + 20, 420+60*i, text, screen);
        }
    }
    //SHOWS YELLOWTILES WHEREEVER IT NEEDS TO
    SDL_Rect yellowtileclip; yellowtileclip.x = 0; yellowtileclip.y = 0; yellowtileclip.w = 150;
    int i;
    if(ZOMBIE_AMOUNT == 5) i=1;
    else if(ZOMBIE_AMOUNT == 10) i=2;
    else if(ZOMBIE_AMOUNT == 20) i=3;
    else if(ZOMBIE_AMOUNT == 25) i=4;
    else if(ZOMBIE_AMOUNT == 30) i=5;
    if(i==5) yellowtileclip.w = 180;
    yellowtileclip.h = 50;
    apply_surface(500 + 150*(i-1), 265, yellowtile, screen, &yellowtileclip);
    yellowtileclip.w = 150;
    if(WIN_AMOUNT == 50) i=1;
    if(WIN_AMOUNT == 100) i=2;
    if(WIN_AMOUNT == 250) i=3;
    if(WIN_AMOUNT == 800) i=4;
    if(WIN_AMOUNT == 0) i=5;
    if(i==5) yellowtileclip.w = 180;
    yellowtileclip.h = 55;
    apply_surface(500 + 150*(i-1), 210, yellowtile, screen, &yellowtileclip);
    yellowtileclip.w = 150;
    i = BOARD_WIDTH/10; yellowtileclip.h = 40; if(i==5) yellowtileclip.w = 180;
    apply_surface(500 + 150*(i-1), 170, yellowtile, screen, &yellowtileclip);
    yellowtileclip.w = 150;
    i = BOARD_HEIGHT/5; yellowtileclip.h = 45; if(i==5) yellowtileclip.w = 180;
    apply_surface(500 + 150*(i-1), 125, yellowtile, screen, &yellowtileclip);
    yellowtileclip.w = 150;
    i = PLAYERS; yellowtileclip.h = 85; if(i==5) yellowtileclip.w = 180;
    apply_surface(500 + 150*(i-1), 40, yellowtile, screen, &yellowtileclip);
    yellowtileclip.w = 150;
    SDL_Flip(screen);
}

void settings() {
    //LOADS ALL THE IMAGES AND FILES
    SDL_Surface *settings_image = load_image("Settings.png");
    SDL_Surface *yellowtile = load_image("bigTiles.png");
    fstream settings_storage("BLOBsettings.txt");
    SDL_Rect yellowtileclip; yellowtileclip.x = 0; yellowtileclip.y = 0;
    bool settingsFinished = false;
    char CONTROLSarray[MAX_PLAYERS][7];
    //GETS ALL THE INFORMATION FOR THE CURRENT GAME FROM FILE
    settings_storage >> PLAYERS >> BOARD_WIDTH >> BOARD_HEIGHT >> WIN_AMOUNT >> ZOMBIE_AMOUNT;
    for(int i=0; i<MAX_PLAYERS; i++)
        for(int j=0; j<7; j++)
            settings_storage >> CONTROLSarray[i][j];
    settingsDisplay(settings_image, yellowtile, CONTROLSarray);
    ///char a[2] = {(char)'n','\0'}; SDL_Surface *text = TTF_RenderText_Solid(fontsmall, a, textColor); apply_surface(100,670,text,screen);SDL_Flip(screen); SDL_Delay(1000);
    while(!settingsFinished) {
        startTick = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState( &x, &y );
                //PLAYER CLICKS OK, CHECKS IF ALL KEYS ARE UNIQUE
                if(x >= 1200 && y >= 660) {
                    bool keysUnique = true;
                    for(int i=0; i<PLAYERS; i++) {
                        for(int j=0; j<7; j++) {
                            for(int k=0; k<PLAYERS; k++) {
                                for(int l=0; l<7; l++) {
                                    if(CONTROLSarray[i][j]==CONTROLSarray[k][l] && !(i==k && j==l)) {
                                        keysUnique = false;
                                        break;
                                    }
                                } if(!keysUnique) break;
                            } if(!keysUnique) break;
                        } if(!keysUnique) break;
                    } if(keysUnique) {
                        settingsFinished = true;
                    } else {
                        SDL_Surface *text = TTF_RenderText_Solid(fontsmall, "Keys aren't unique. Try changing them and exitting again.", textColor); apply_surface(100,670,text,screen);SDL_Flip(screen); SDL_Delay(1000);
                    }
                //PLAYER IS CHANGING ONE OF THE CONTROLS
                } else if(x >= 180 && x <= 810 && y >= 420 && y <= 660) {
                    int j = (x-180)/90;
                    int i = (y-420)/60;
                    yellowtileclip.w = 90;
                    yellowtileclip.h = 60;
                    apply_surface(j*90+180, i*60+420, yellowtile, screen, &yellowtileclip); SDL_Flip(screen);
                    bool keyEntered = false;
                    while(!keyEntered) {
                        startTick = SDL_GetTicks();
                        while(SDL_PollEvent(&event)) if(event.type==SDL_KEYDOWN) { CONTROLSarray[i][j] = (char)event.key.keysym.sym; keyEntered = true; }
                        endTick = SDL_GetTicks();
                        frameTicks = endTick - startTick;
                        frameRegulate(frameTicks);
                    }
                //PLAYER IS EDITTING SETTINGS (CODE IS SELF-DOCUMENTING)
                } else if(x >= 500 && y <=315) {
                    int i = (x-500)/150 + 1;
                    if(y >= 265) {
                        if(i == 1) ZOMBIE_AMOUNT = 5;
                        else if(i == 2) ZOMBIE_AMOUNT = 10;
                        else if(i == 3) ZOMBIE_AMOUNT = 20;
                        else if(i == 4) ZOMBIE_AMOUNT = 25;
                        else if(i == 5) ZOMBIE_AMOUNT = 30;
                    } else if(y >= 210) {
                        if(i == 1) WIN_AMOUNT = 50;
                        else if(i == 2) WIN_AMOUNT = 100;
                        else if(i == 3) WIN_AMOUNT = 250;
                        else if(i == 4) WIN_AMOUNT = 800;
                        else if(i == 5) WIN_AMOUNT = 0;
                    } else if(y >= 170) {
                        BOARD_WIDTH = 10*i;
                    } else if(y >= 125) {
                        BOARD_HEIGHT = 5*i;
                    } else if(y >= 40 && x <= 1100) {
                        PLAYERS = i;
                    }
                }
            }
            settingsDisplay(settings_image, yellowtile, CONTROLSarray);
        }
        endTick = SDL_GetTicks();
        frameTicks = endTick - startTick;
        frameRegulate(frameTicks);
    }
    //CLOSES FILE AND REOPENS TO STORE ALL THE NEW DATA
    settings_storage.close(); settings_storage.open("BLOBsettings.txt");
    settings_storage << PLAYERS << ' ' << BOARD_WIDTH << ' ' << BOARD_HEIGHT << ' ' << WIN_AMOUNT << ' ' << ZOMBIE_AMOUNT << '\n';
    for(int i=0; i<MAX_PLAYERS; i++) {
        for(int j=0; j<7; j++) {
            char control[2] = {CONTROLSarray[i][j],'\0'};
            settings_storage << control << ' ';
        }
        settings_storage << endl;
    }
    settings_storage.close();
}

void help() {
    //LOADS THE HELP SCREEN IMAGE
    clearScreen();
    SDL_Surface *help_image = load_image("help.png");
    apply_surface(0, 0, help_image, screen); SDL_Flip(screen);
    bool helpFinished = false;
    while(!helpFinished) {
        startTick = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_MOUSEBUTTONDOWN) {
                int x,y;
                SDL_GetMouseState(&x,&y);
                //SHORT TUTORIAL IF USER CLICKS ON BUTTONS (SPECIFIED BY COORDINATES BELOW)
                if(x >= 1130 && y >= 25 && x <= 1271 && y <= 150) {
                    GAME g;
                    g.gameType = 1; //PLAYING AGAINST 1 AI
                    PLAYERS = 2; WIN_AMOUNT = 50;
                    g.INITreactionTime = 0.3 * FRAMES_PER_SECOND;
                    g.play();
                } else if(x >= 1130 && y >= 200 && x <= 1271 && y <= 325) {
                    GAME g;
                    g.gameType = 2; //PLAYING CAPTURE THE FLAG
                    PLAYERS = 2; WIN_AMOUNT = 50;
                    g.play();
                } else if(x >= 1130 && y >= 380 && x <= 1271 && y <= 505) {
                    GAME g;
                    g.gameType = 3; //PLAYING ZOMBIES MODE
                    WIN_AMOUNT = 50;
                    ZOMBIE_AMOUNT = 5;
                    g.play();
                } else {
                    helpFinished = true;
                }
            }
            if(!helpFinished) {
                clearScreen();
                SDL_Surface *help_image = load_image("help.png");
                apply_surface(0, 0, help_image, screen); SDL_Flip(screen);
            }
        }
        endTick = SDL_GetTicks();
        frameTicks = endTick - startTick;
        frameRegulate(frameTicks);
    }
}

int main( int argc, char* argv[] ) {
    //STARTS SDL AND LOADS ALL THE NECESSARY FILES
    music = true;
    quit = false;
    if (!init() || !load_font())
        return 1;
    set_clips();
	pallette = load_image("pallette.png");
    bombChunk = Mix_LoadWAV("bomb.wav");
    camoChunk = Mix_LoadWAV("camo.wav");
    trapChunk = Mix_LoadWAV("trap.wav");
    pointChunk = Mix_LoadWAV("point.wav");
    winChunk = Mix_LoadWAV("win.wav");
    backgroundMusic = Mix_LoadMUS("startBackgroundMusic.wav");
    inGameMusic = Mix_LoadMUS("inGameMusic.wav");

    SDL_Surface *gametypeSurface = load_image("startTiles.png");
    SDL_Rect gametypeTiles = {0, 0, 500, 160};
    SDL_Rect musicOnOffTile = {0, 160, 140, 100};
    if(music)
        Mix_PlayMusic(backgroundMusic, -1);
    while(!quit) {
        startTick = SDL_GetTicks();
        while( SDL_PollEvent( &event ) ) {
            clearScreen();
            SDL_Surface *start = load_image("start.png");
            apply_surface(0,0,start, screen);
            apply_surface(820, 590, gametypeSurface, screen, &musicOnOffTile); SDL_Flip(screen);
            //CHECKS WHICH BOX THE USER CLICKS ON (ie, EXIT, SETTINGS, HELP, PLAY)
            if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                int x, y;
                SDL_GetMouseState( &x, &y );
                if(x>=1230 && y >= 670) {
                    quit = true;
                } else if(x>=90 && y>=315 && x<=610 && y<=435) {
                    apply_surface(620, 315, gametypeSurface, screen, &gametypeTiles); SDL_Flip(screen);
                    bool gametypeSelected = false;
                    while(!gametypeSelected) {
                        startTick = SDL_GetTicks();
                        while(SDL_PollEvent(&event)) {
                            if(event.type==SDL_MOUSEBUTTONDOWN) {
                                gametypeSelected = true;
                                int x,y;
                                SDL_GetMouseState(&x,&y);
                                //SELECT THE GAMEMODE AND PLAY THE GAME
                                if(x >= 620 && y >= 315 && x < 745 && y <= 475) {
                                    GAME g;
                                    g.gameType = 1; //PLAYER V CPU
                                    g.play();
                                } else if(x >= 745 && y >= 315 && x < 870 && y <= 475) {
                                    GAME g;
                                    g.gameType = 0; //PLAYER V PLAYER
                                    g.play();
                                } else if(x >= 870 && y >= 315 && x < 995 && y <= 475) {
                                    GAME g;
                                    g.gameType = 2; //CAPTURE THE FLAG
                                    g.play();
                                } else if(x >= 995 && y >= 315 && x < 1120 && y <= 475) {
                                    GAME g;
                                    g.gameType = 3; //PLAYER V ZOMBIES
                                    g.play();
                                }
                            }
                        }
                        endTick = SDL_GetTicks();
                        frameTicks = endTick - startTick;
                        frameRegulate(frameTicks);
                    }
                } else if(x>=90 && y>=452 && x<=1075 && y<=572) {
                    settings();
                } else if(x>=90 && y>=590 && x<=610 && y<=710) {
                    help();
                } else if(x>=820 && y>=590 && x<=960 && y<=690) {
                    if(music) {
                        Mix_HaltMusic();
                        music = false;
                        musicOnOffTile.x = 140;
                    } else {
                        Mix_PlayMusic(backgroundMusic, -1);
                        music = true;
                        musicOnOffTile.x = 0;
                    }
                }
            }
        }
        endTick = SDL_GetTicks();
        frameTicks = endTick - startTick;
        frameRegulate(frameTicks);
    }
    clean_up();
    return 0;
}
