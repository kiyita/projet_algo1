#include <stdbool.h> // bool, true, false
#include <stdlib.h>  // rand
#include <stdio.h>   // printf
#include "lode_runner.h" // types and prototypes used to code the game
#include <math.h> // math operation
#include <assert.h> // assert

// global declarations used by the game engine
extern const char BOMB;  // ascii used for the bomb
extern const char BONUS;  // ascii used for the bonuses
extern const char CABLE;  // ascii used for the cable
extern const char ENEMY;  // ascii used for the ennemies
extern const char EXIT;   // ascii used for the exit
extern const char FLOOR;  // ascii used for the floor
extern const char LADDER; // ascii used for the ladder
extern const char PATH;   // ascii used for the pathes
extern const char RUNNER; // ascii used for runner
extern const char WALL;   // ascii used for the walls

extern const int BOMB_TTL; // time to live for bombs

extern bool DEBUG; // true if and only if the game runs in debug mode

const char *students = "Bihet"; // replace Radom with the student names here

// local prototypes (add your own prototypes below)
void print_action(action); 

/* 
  function to code: it may use as many modules (functions and procedures) as needed
  Input (see lode_runner.h for the type descriptions): 
    - level provides the information for the game level
    - characterl is the linked list of all the characters (runner and enemies)
    - bonusl is the linked list of all the bonuses that have not been collected yet
    - bombl is the linked list of all the bombs that are still active
  Output
    - the action to perform
*/

typedef struct Coord { // structure for coordinates
  int x;
  int y;
  struct Coord* suivant;
} coord;

typedef struct File { // structure for FIFO
  coord *debut;
  coord *fin;
} file;

file *creerFile() { // initialize an empty FIFO
  file *a = malloc(sizeof(file));
  a->debut = NULL;
  a->fin = NULL;
  return a;
}

bool estVideFile(file *f) { // true if FIFO is empty
  return f->debut == NULL;
}

void enfile(file *f, coord d) { // put a tail
  coord *e = malloc(sizeof(coord));
  e->x = d.x;
  e->y = d.y;
  e->suivant = NULL;
  if (estVideFile(f)) {
    f->debut = e;
  } else {
    f->fin->suivant = e;
  }
  f->fin = e;
}

coord defile(file *f) { // take the head
  assert(!estVideFile(f));
  coord *a = f->debut;
  coord v = *a;
  f->debut = a->suivant;
  free(a);
  if (f->debut == NULL) {
    f->fin = NULL;
  }
  return v;
}


int testVoisins(int x, int y, char **map, file *f, coord liste[35][35]) { //put in FIFO all possible path from x,y
  //printf("test voisins /n"); //marqueur erreur

  coord temp;

  if (map[y][x+1] != WALL && liste[y][x+1].x == -1) { //each test
    temp.x = x+1; temp.y = y;
    enfile(f, temp);
    liste[y][x+1] = temp;
  }
  if (map[y][x-1] != WALL && liste[y][x-1].x == -1) {
    temp.x = x-1; temp.y = y;
    enfile(f, temp);
    liste[y][x-1] = temp;
  }
  if ((map[y+1][x] == LADDER || map[y][x+1] == PATH) && liste[y+1][x].x == -1) {
    temp.x = x; temp.y = y+1;
    enfile(f, temp);
    liste[y+1][x] = temp;
  }
  if (map[y-1][x] == LADDER  && liste[y-1][x].x == -1) {
    temp.x = x; temp.y = y-1;
    enfile(f, temp);
    liste[y+1][x] = temp;
  }
  return 0;
}

action bonusTrouve(int x, int y, int xb, int yb, coord liste[35][35]) { //retrace the path when a bonus is found
  //printf("bonus trouve /n"); //marqueur erreur
  while (liste[yb][xb].x != x && liste[yb][xb].x != y) { //back in the "tree"
    xb = liste[yb][xb].x; yb = liste[yb][xb].y;
  }
  if (xb == x+1) return RIGHT; //give the right action
  if (xb == x-1) return LEFT;
  if (yb == y+1) return DOWN;
  if (yb == y-1) return UP;
  else {
    printf("oops /n"); //marqueur erreur
    return NONE;
  }
}

action ParcoursLargeur(char **map, int x, int y) { //BFS
  //printf("parcours largeur /n"); //marqueur erreur
  coord en_cours; // to mark
  coord liste[35][35];
  file *f = creerFile();

  for (int i = 0; i<35; i++) { //put -1 in the marking tab
    for (int j = 0; j < 35; j++) {
      liste[i][j] = (coord) {.x = -1, .y = -1};
    }
  }

  en_cours.x = x; en_cours.y = y; // coord visiting now
  enfile(f, en_cours);
  liste[y][x] = en_cours;

  action a;
  while (!estVideFile(f)) { //BFS
    en_cours = defile(f);
    if (map[en_cours.y][en_cours.x] == BONUS) { // if bonus found go in the function
      a = bonusTrouve(x, y, en_cours.x, en_cours.y, liste);
    }
    else {
      testVoisins(en_cours.x, en_cours.y, map, f, liste);
    }
  }
  return a;
}

action next_step(int x, int y, char **map) { // choose next action based on coord initial and final
  //printf("quoicoubeh (next step) /n"); //marqueur erreur
  action a = ParcoursLargeur(map, x, y);
  return a;
}


action lode_runner(
  levelinfo level,
  character_list characterl,
  bonus_list bonusl,
  bomb_list bombl
  )
{
  //printf("aaaaaah /n"); //marqueur erreur
  action a; // action to choose and then return
  bool ok; // boolean to control the do while loops
  
  int x; // runner's x position
  int y; // runner's y position

  character_list pchar=characterl; // iterator on the character list

  // looking for the runner ; we know s.he is in the list
  ok=false; // ok will become true when the runner will be found
  do
  { 
    if(pchar->c.item==RUNNER) // runner found
    {
      x=pchar->c.x; 
      y=pchar->c.y;
      ok=true;
    }
    else // otherwise move on next character
      pchar=pchar->next;
  } while(!ok);

  ok=false; // ok will become true when a valid action will be guessed
  do
  {
    printf("x:%d et y:%d; ", x, y);
    printf("block:%c \n", level.map[y][x]);
    //printf("début /n"); //marqueur erreur
    a = next_step(x, y, level.map);
    ok = true;
    
    if(DEBUG) // only when the game is in debug mode
    {
      printf("[Player] Candidate action ");
      print_action(a);
      if(ok) 
        printf(" is valid"); 
      else 
        printf(" not valid");
      printf(".\n");
    }
  } while (!ok);

  return a; // action to perform
}

/*
  Procedure that print the action name based on its enum type value
  Input:
    - the action a to print
*/
void print_action(action a)
{
  switch (a)
  {
  case NONE:
    printf("NONE");
    break;
  case UP:
    printf("UP");
    break;
  case DOWN:
    printf("DOWN");
    break;
  case LEFT:
    printf("LEFT");
    break;
  case RIGHT:
    printf("RIGHT");
    break;
  case BOMB_LEFT:
    printf("BOMB_LEFT");
    break;
  case BOMB_RIGHT:
    printf("BOMB_RIGHT");
    break;
  }
}