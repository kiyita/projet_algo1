#include <stdbool.h> 
#include <stdlib.h>  
#include <stdio.h>   
#include <assert.h>  
#include "lode_runner.h" 

// Global constants
extern const char BOMB, BONUS, CABLE, ENEMY, EXIT, FLOOR, LADDER, PATH, RUNNER, WALL;
extern const int BOMB_TTL;
extern bool DEBUG;

const char *students = "Lucie Bihet";

/////////////////////////////////////////////////////////////////////////////////////////////////
// Coordinates
/////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Coord {
    //coordinates structure with a cost and a pointer to the next element
    int x;
    int y;
    int cost;
    struct Coord* next;
} coord;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Files
/////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct File { 
    //file structure with a pointer to the first element
    coord *start;
} file;

file *creerFile() {
    //creates a file
    file *a = malloc(sizeof(file));
    a->start = NULL;
    return a;
}

bool estVideFile(file *f) { 
    //checks if the file is empty
    return f->start == NULL;
}

void enfilePrioritaire(file *f, coord d) { 
    //adds an element to the file with a priority
    coord *e = malloc(sizeof(coord));
    e->x = d.x;
    e->y = d.y;
    e->cost = d.cost;
    e->next = NULL;
    if (estVideFile(f)) {
        f->start = e;
    } else {
        coord *a = f->start;
        while (a->next != NULL && a->next->cost <= e->cost) {
            a = a->next;
        }
        coord *temp = a->next;
        a->next = e;
        e->next = temp;
    }
}

void enfile(file *f, coord d) { 
    //adds an element to the file without priority
    coord *e = malloc(sizeof(coord));
    e->x = d.x;
    e->y = d.y;
    e->next = NULL;
    if (estVideFile(f)) {
        f->start = e;
    } else {
        coord *a = f->start;
        while (a->next != NULL) {
            a = a->next;
        }
        coord *temp = a->next;
        a->next = e;
        e->next = temp;
    }
}

coord defile(file *f) { 
    //removes the first element of the file
    assert(!estVideFile(f));
    coord *a = f->start;
    coord v = *a;
    f->start = a->next;
    free(a);
    return v;
}

int dansFile(file *f, int x, int y) {
    //checks if an element is in the file and returns its cost if it is in the file or -1 if it is not
    coord *a = f->start;
    while (a != NULL) {
        if (a->x == x && a->y == y) {
            return a->cost;
        }
        a = a->next;
    }
    return -1;
}

void retirerFile(file *f, int x, int y) {
    //removes an element from the file
    coord *a = f->start;
    coord *temp = NULL;
    while (a != NULL) {
        if (a->x == x && a->y == y) {
            if (temp == NULL) {
                f->start = a->next;
            } else {
                temp->next = a->next;
            }
            free(a);
            break;
        }
        temp = a;
        a = a->next;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////////////////////////

//print the action
void print_action(action); 

//free the memory of a matrix
void freeMatrix(char **matrix, int ysize);
//free the memory of a file
void freeFile(file *f);

//check if the player can move to the right
bool checkRight(char **map, int x_player, int y_player);
//check if the player can move to the left
bool checkLeft(char **map, int x_player, int y_player);
//check if the player can move up
bool checkUp(char **map, int x_player, int y_player);
//check if the player can move down
bool checkDown(char **map, int x_player, int y_player);
//check if the player can place a bomb to the left
bool checkBombLeft(char **map, int x_player, int y_player);
//check if the player can place a bomb to the right
bool checkBombRight(char **map, int x_player, int y_player);

//check if an enemy is at a Manhattan distance of 2
bool isEnemyNear(char **map, int x, int y, levelinfo level);
//Manage when an enemy is bit too close
action perduPourPerdu(char **map, int x, int y, levelinfo level);

//Possible moves, with checks for the BFS algorithm
void checkPaths(char **map, int x, int y, coord **list_parents, file *f);
//Retrace the path from a case to the bonus to mark the distance to the bonus for each case
void retraceRouteForBFS(coord **list_parents, int x_start, int y_start, int x_bonus, int y_bonus, int **heuristic);
//Calculate the heuristic for each case with a BFS algorithm
void BFS(char **map, int **heuristic, int x_start, int y_start, levelinfo level);
//add a danger cost to the heuristic table

void updateDangerHeuristic(char **map, int **heuristic, levelinfo level);
//calculate the heuristic for each case with the BFS algorithm
void heuristicCalculation(levelinfo level, char **map, int **heuristic, bonus_list bonusl);

//process a neighbor case for the testNeigbors for the A* algorithm
void processNeighbor(int x, int y, int x_neighbor, int y_neighbor, int **g, int **f, coord **list_parents, file *openFile, file *closedFile, int **heuristic);
//test if a neighbor is a accessible path for the A* algorithm
void testNeighbors(char **map, int x, int y, int **g, int **f, coord **list_parents, file *openFile, file *closedFile, int **heuristic);
//retrace the path for the A* algorithm
action retracePathForAStar(int x_depart, int y_depart, int x_target, int y_target, coord **list_parents, levelinfo level);
//A* algorithm
action aStarAlgorithm(char **map, int x, int y, bonus_list bonusl, levelinfo level);

//main function, find the coordinates of th eplayer, put the bonus, the bombs and the ennemies on the map and launch the A* algorithm
action lode_runner(levelinfo level, character_list characterl, bonus_list bonusl, bomb_list bombl);



/////////////////////////////////////////////////////////////////////////////////////////////////
// Free memory
/////////////////////////////////////////////////////////////////////////////////////////////////

void freeMatrix(char **matrix, int ysize) {
    //free the memory of a matrix

    for (int i = 0; i < ysize; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void freeFile(file *f) {
    //free the memory of a file

    while (!estVideFile(f)) {
        defile(f);
    }
    free(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Moves tests
/////////////////////////////////////////////////////////////////////////////////////////////////

bool checkRight(char **map, int x_player, int y_player) {
    // Check if the player can move to the right

    if ((map[y_player][x_player + 1] != WALL && map[y_player][x_player + 1] != FLOOR) && //Check all conditions to not make a forbidden move
        (map[y_player + 1][x_player] != PATH || map[y_player - 1][x_player] == CABLE )) { 
        if (map[y_player + 1][x_player + 1] == BOMB && map[y_player + 2][x_player + 1] == WALL) {
            return false;
        }
        if (map[y_player + 1][x_player + 1] == PATH) { //Check if you can fall without an enemy killing you right away
            int i = 0;
            while (map[y_player + 1 + i][x_player + 1] == PATH) {
                i++;
            }
            if (map[y_player + 2 + i][x_player] == ENEMY || map[y_player + 2 + i][x_player + 2] == ENEMY) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool checkLeft(char **map, int x_player, int y_player) {
    // Check if the player can move to the left

    if ((map[y_player][x_player - 1] != WALL && map[y_player][x_player - 1] != FLOOR) && 
        (map[y_player + 1][x_player] != PATH || map[y_player - 1][x_player] == CABLE )) {
            if (map[y_player + 1][x_player - 1] == BOMB && map[y_player + 2][x_player - 1] == WALL) {
                return false;
            }
            if (map[y_player + 1][x_player - 1] == PATH) {
                int i = 0;
                while (map[y_player + 1 + i][x_player - 1] == PATH) {
                    i++;
                }
                if (map[y_player + 2 + i][x_player] == ENEMY || map[y_player + 2 + i][x_player - 2] == ENEMY) {
                    return false;
                }
            }
            return true;
        }
    return false;
}

bool checkUp(char **map, int x_player, int y_player) {
    // Check if the player can move up

    if (map[y_player][x_player] == LADDER && map[y_player - 1][x_player] != WALL && map[y_player - 1][x_player] != FLOOR)
        return true;
    else return false;
}

bool checkDown(char **map, int x_player, int y_player) {
    // Check if the player can move down

    if ((map[y_player + 1][x_player] != WALL && map[y_player + 1][x_player] != FLOOR)
        && (map[y_player + 1][x_player] == PATH || map[y_player + 1][x_player] == LADDER || map[y_player + 1][x_player] == CABLE))
        return true;
    else return false;
}

bool checkBombLeft(char **map, int x_player, int y_player) {
    // Check if the player can place a bomb to the left

    if (map[y_player][x_player - 1] != WALL && map[y_player][x_player - 1] != FLOOR && map[y_player + 1][x_player - 1] == FLOOR  && map[y_player][x_player - 1] != LADDER)
        return true;
    else return false;
}

bool checkBombRight(char **map, int x_player, int y_player) {
    // Check if the player can place a bomb to the right

    if (map[y_player][x_player + 1] != WALL && map[y_player][x_player + 1] != FLOOR && map[y_player + 1][x_player + 1] == FLOOR && map[y_player][x_player + 1] != LADDER)
        return true;
    else return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Enemy management
/////////////////////////////////////////////////////////////////////////////////////////////////

bool isEnemyNear(char **map, int x, int y, levelinfo level) {
    //check if an enemy is at a Manhattan distance of 2
    
    for (int i = -2; i < 3; i++) {
        for (int j = -2; j < 3; j++) {
            if (abs(i) + abs(j) <= 2 && x + i >= 0 && x + i < level.xsize && y + j >= 0 && y + j < level.ysize) {
                if (map[y + j][x + i] == ENEMY) {
                    return true;
                }
            }
        }
    }
    return false;
}

action perduPourPerdu (char **map, int x, int y, levelinfo level) {
    //Manage when an enemy is bit too close

    //Check if an enemy is 2 cases away and put a bomb in that direction
    if (x + 2 < level.xsize && map[y][x + 2] == ENEMY && checkBombRight(map, x, y)) {
        return BOMB_RIGHT;
    }
    if (x - 2 >= 0 && map[y][x - 2] == ENEMY && checkBombLeft(map, x, y)) {
        return BOMB_LEFT;
    }
    
    //Check if there is no enemy 3 cases away and move in that direction to ensure a safe escape
    if (y + 3 < level.ysize && map[y + 1][x] != ENEMY && map[y + 2][x] != ENEMY && map[y + 3][x] != ENEMY && checkDown(map, x, y)) {
        if (map[y + 1][x + 1] != ENEMY && map[y + 1][x - 1] != ENEMY) {
            return DOWN;
        }
    }
    if (y - 3 >= 0 && map[y - 1][x] != ENEMY && map[y - 2][x] != ENEMY && map[y - 3][x] != ENEMY && checkUp(map, x, y)) {
        if (map[y - 1][x + 1] != ENEMY && map[y - 1][x - 1] != ENEMY) {
            return UP;
        }
    }
    if (x + 3 < level.xsize && map[y][x + 1] != ENEMY && map[y][x + 2] != ENEMY && map[y][x + 3] != ENEMY && checkRight(map, x, y)) {
        if (map[y + 1][x + 1] != ENEMY && map[y - 1][x + 1] != ENEMY) {
            return RIGHT;
        }
    }
    if (x - 3 >= 0 && map[y][x - 1] != ENEMY && map[y][x - 2] != ENEMY && map[y][x - 3] != ENEMY && checkLeft(map, x, y)) {
        if (map[y + 1][x - 1] != ENEMY && map[y - 1][x - 1] != ENEMY) {
            return LEFT;
        }
    }

    //If you are in a situation where you can put a bomb, do it
    else {
        for (int i = 1; i < level.xsize; i++) {
            if (x - i >= 0 && map[y][x - i] == ENEMY && checkBombLeft(map, x, y)) {
                return BOMB_LEFT;
            }
            if (x + i < level.xsize && map[y][x + i] == ENEMY && checkBombRight(map, x, y)) {
                return BOMB_RIGHT;
            }
        }
    }

    //Else, check all possible escape routes
    if (checkUp(map, x, y) && map[y - 1][x] != ENEMY && map[y - 1][x + 1] != ENEMY && map[y - 1][x - 1] != ENEMY) return UP;
    if (checkDown(map, x, y) && map[y + 1][x] != ENEMY && map[y + 1][x + 1] != ENEMY && map[y + 1][x - 1] != ENEMY) return DOWN;
    if (checkLeft(map, x, y) && map[y][x - 1] != ENEMY && map[y + 1][x - 1] != ENEMY && map[y - 1][x - 1] != ENEMY) return LEFT;
    if (checkRight(map, x, y) && map[y][x + 1] != ENEMY && map[y + 1][x + 1] != ENEMY && map[y - 1][x + 1] != ENEMY) return RIGHT;

    //You are dead
    return NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// BFS pour chaque case (pour l'heuristique)
/////////////////////////////////////////////////////////////////////////////////////////////////

void checkPaths(char **map, int x, int y, coord **list_parents, file *f) {
    // Possible moves, with checks

    // Move to the right
    if (checkRight(map, x, y) && list_parents[y][x + 1].x == -1) {
        list_parents[y][x + 1] = (coord){.x = x, .y = y};
        enfile(f, (coord){.x = x + 1, .y = y});
    }

    // Move to the left
    if (checkLeft(map, x, y) && list_parents[y][x - 1].x == -1) {
        list_parents[y][x - 1] = (coord){.x = x, .y = y};
        enfile(f, (coord){.x = x - 1, .y = y});
    }

    // Move down
    if (checkDown(map, x, y) && list_parents[y + 1][x].x == -1) {
        list_parents[y + 1][x] = (coord){.x = x, .y = y};
        enfile(f, (coord){.x = x, .y = y + 1});
    }

    // Move up
    if (checkUp(map, x, y) && list_parents[y - 1][x].x == -1) {
        list_parents[y - 1][x] = (coord){.x = x, .y = y};
        enfile(f, (coord){.x = x, .y = y - 1});
    }
}


void retraceRouteForBFS(coord **list_parents, int x_start, int y_start, int x_bonus, int y_bonus, int **heuristic) {
    // Retrace the path from a case to the bonus to mark the distance to the bonus for each case

    heuristic[y_bonus][x_bonus] = 0;
    int path_counter = 1;
    while (!(list_parents[y_bonus][x_bonus].x == x_start && list_parents[y_bonus][x_bonus].y == y_start)) {
    // Save data in temporary variables
        int x_temp = list_parents[y_bonus][x_bonus].x;
        int y_temp = list_parents[y_bonus][x_bonus].y;
        // Update xb and yb
        x_bonus = x_temp;
        y_bonus = y_temp;
        //add the path counter to the case
        heuristic[y_bonus][x_bonus] = path_counter;
        path_counter += 1;
        }
    heuristic[y_start][x_start] = path_counter;
}

void BFS(char **map, int **heuristic, int x_start, int y_start, levelinfo level) {
    //Calculate the heuristic for each case with a BFS algorithm
    //Give each case the distance to the nearest bonus

    coord ongoing;//init the variable for the case visiting
    coord **list_parents = malloc((size_t)level.ysize * sizeof(coord *));//init the table of parents
    for (int i = 0; i < level.ysize; i++) {
        list_parents[i] = malloc((size_t)level.xsize * sizeof(coord));
        for (int j = 0; j < level.xsize; j++) {
            list_parents[i][j] = (coord){.x = -1, .y = -1};//init the table with -1 as coordinates
        }
    }
    file *f = creerFile();//init the file


    ongoing.x = x_start; //init the starting case
    ongoing.y = y_start;
    list_parents[y_start][x_start] = ongoing;//mark the starting case as visited and parent of itself
    enfile(f, ongoing); //enfile the starting case

    while (!estVideFile(f)) {//BFS algorithm
        ongoing = defile(f);//defile the first case
        if (map[ongoing.y][ongoing.x] == BONUS) {//if the case is a bonus
            retraceRouteForBFS(list_parents, x_start, y_start, ongoing.x, ongoing.y, heuristic);//retrace the path to the bonus to mark the distance to the bonus for each case
            break;
        }
        checkPaths(map, ongoing.x, ongoing.y, list_parents, f);//dd the possible paths to the file
    }
    freeFile(f);//free the memory of the file
    freeMatrix((char **)list_parents, level.ysize);//free the memory of the parents tabel
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Heuristic
/////////////////////////////////////////////////////////////////////////////////////////////////

void updateDangerHeuristic(char **map, int **heuristic, levelinfo level) {
    //add a danger cost to the heuristic table
    //calculate with +20 - 2*distance

    for (int i = 0; i < level.ysize; i++) {
        for (int j = 0; j < level.xsize; j++) {
            if (map[i][j] == ENEMY) {
                for (int k = -3; k < 4; k++) {
                    for(int l = -3; l < 4; l++) {
                        int distance = abs(k) + abs(l);
                        if (i + k >= 0 && i + k < level.ysize && j + l >= 0 && j + l < level.xsize) {
                                heuristic[i + k][j + l] += 20 - 2*distance;
                        }
                    }
                }
            }
        }
    }
}

void heuristicCalculation(levelinfo level, char **map, int **heuristic, bonus_list bonusl) {
    //calculate the heuristic for each case with the BFS algorithm

    for (int i = 0; i < level.ysize; i++) {//allocate memory for the heuristic table
        heuristic[i] = malloc((size_t)level.xsize * sizeof(int));
    }
    for (int i = 0; i < level.ysize; i++) {//init the table with 999
        for (int j = 0; j < level.xsize; j++) {
            heuristic[i][j] = 999;
        }
    }
    
    if (bonusl != NULL) {//if there is a bonus, launch the BFS algorithm
        for (int i = 1; i < level.ysize; i++) {//lauch the BFS for each case not visited yet
            for (int j = 1; j < level.xsize; j++) {
                    if (map[i][j] != WALL && map[i][j] != FLOOR && heuristic[i][j] >= 999) {
                        BFS(map, heuristic, j, i, level);
                }
            }
        }
    }

    updateDangerHeuristic(map, heuristic, level);//update the danger heuristic
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Subfunctions de A* (processNeighbor, testNeighbors, retracePathForAStar)
/////////////////////////////////////////////////////////////////////////////////////////////////

void processNeighbor(int x, int y, int x_neighbor, int y_neighbor, int **g, int **f, coord **list_parents, file *openFile, file *closedFile, int **heuristic) {
    //process a neighbor case for the testNeigbors for the A* algorithm

    int g_tmp = g[y][x] + 1; //calculate the cost g which the cost of the "parent" + 1
    int h = heuristic[y_neighbor][x_neighbor];
    int f_tmp = g_tmp + h; //calculate the cost f which is the cost g + the heuristic
    int f_cost_open = dansFile(openFile, x_neighbor, y_neighbor); //check if the case is in the file and if yes get the cost
    int f_cost_closed = dansFile(closedFile, x_neighbor, y_neighbor);

    if ((f_cost_open != -1 && f_tmp > f_cost_open) || (f_cost_closed != -1 && f_tmp > f_cost_closed)) { //if the case is in the file and the cost is lower, do nothing
        return;
    }

    list_parents[y_neighbor][x_neighbor] = (coord){.x = x, .y = y}; //mark the parent of the case
    g[y_neighbor][x_neighbor] = g_tmp; //update the cost g
    f[y_neighbor][x_neighbor] = f_tmp; //update the cost f

    if (f_cost_closed != -1) { //if the case is in the closed file, remove it
        retirerFile(closedFile, x_neighbor, y_neighbor);
    }
    if (f_cost_open != -1) { //if the case is in the open file, remove it
        retirerFile(openFile, x_neighbor, y_neighbor);
    }
    enfilePrioritaire(openFile, (coord){.x = x_neighbor, .y = y_neighbor, .cost = f_tmp}); //enfilePrioritaire the case with the cost f
}

void testNeighbors(char **map, int x, int y, int **g, int **f, coord **list_parents, file *openFile, file *closedFile, int **heuristic) {
    //test if the neighbors of a case are accessible for the A* algorithm

    if (checkRight(map, x, y) && list_parents[y][x + 1].x == -1) { // if the case is accessible and not visited yet
        processNeighbor(x, y, x + 1, y, g, f, list_parents, openFile, closedFile, heuristic); //process the case
    }
    if (checkLeft(map, x, y) && list_parents[y][x - 1].x == -1) {
        processNeighbor(x, y, x - 1, y, g, f, list_parents, openFile, closedFile, heuristic);
    }
    if (checkDown(map, x, y) && list_parents[y + 1][x].x == -1) {
        processNeighbor(x, y, x, y + 1, g, f, list_parents, openFile, closedFile, heuristic);
    }
    if (checkUp(map, x, y) && list_parents[y - 1][x].x == -1) {
        processNeighbor(x, y, x, y - 1, g, f, list_parents, openFile, closedFile, heuristic);
    }
}

action retracePathForAStar(int x_depart, int y_depart, int x_target, int y_target, coord **list_parents, levelinfo level) {
    //retrace the path from a case to the player to make the right move

    while (!(list_parents[y_target][x_target].x == x_depart && list_parents[y_target][x_target].y == y_depart)) {
    // Save data in temporary variables
        int x_temp = list_parents[y_target][x_target].x;
        int y_temp = list_parents[y_target][x_target].y;
        // Update x and y
        x_target = x_temp;
        y_target = y_temp;
        }
    freeMatrix((char **)list_parents, level.ysize);//free teh memory for the parent table
    // Check which direction the playr must take according to the path choosen
    if (x_target == x_depart + 1 && y_target == y_depart) return RIGHT;
    if (x_target == x_depart - 1 && y_target == y_depart) return LEFT;
    if (y_target == y_depart + 1 && x_target == x_depart) return DOWN;
    if (y_target == y_depart - 1 && x_target == x_depart) return UP;
    return NONE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// A*
/////////////////////////////////////////////////////////////////////////////////////////////////

action aStarAlgorithm(char **map, int x, int y, bonus_list bonusl, levelinfo level) {
    
    int **heuristic = malloc((size_t)level.ysize * sizeof(int *)); // allocate memory for the heuristic array;
    heuristicCalculation(level, map, heuristic, bonusl); // calculate the heuristic array


    file *openFile = creerFile();//init the open file
    file *closedFile = creerFile();//and the closed file

    int **tab_g = malloc((size_t)level.ysize * sizeof(int *));//init the cost tables g
    int **tab_f = malloc((size_t)level.ysize * sizeof(int *));//init the cost tables f
    for (int i = 0; i < level.ysize; i++) {
        tab_g[i] = malloc((size_t)level.xsize * sizeof(int));
        tab_f[i] = malloc((size_t)level.xsize * sizeof(int));
    }

    for (int i = 0; i < level.ysize; i++) { //init the cost tables f and g with 0
        for (int j = 0; j < level.xsize; j++) {
            tab_g[i][j] = 0;
            tab_f[i][j] = 0;
        }
    }

    coord ongoing;//init the variable for the case visiting
    ongoing.x = x; 
    ongoing.y = y;
    enfile(openFile, ongoing);//enfile the starting case

    coord **list_parents = malloc((size_t)level.ysize * sizeof(coord *));
    for (int i = 0; i < level.ysize; i++) {
        list_parents[i] = malloc((size_t)level.xsize * sizeof(coord));
        for (int j = 0; j < level.xsize; j++) {
            list_parents[i][j] = (coord){.x = -1, .y = -1};
        }
    }
    list_parents[y][x] = ongoing;//mark the starting case as visited and parent of itself

    if (isEnemyNear(map, x, y, level)) {//if an enemy is near (2 cases away with Manhattan distance)
        freeFile(openFile);//free all the things that need to be freed
        freeFile(closedFile);
        freeMatrix((char **)tab_g, level.ysize);
        freeMatrix((char **)tab_f, level.ysize);
        freeMatrix((char **)heuristic, level.ysize);
        freeMatrix((char **)list_parents, level.ysize);
        return perduPourPerdu(map, x, y, level); //use the urgency function
    }

    while(!estVideFile(openFile)) {//main loop of the A* algorithm
        ongoing = defile(openFile);//défile the case to visit

        if (ongoing.x == level.xexit && ongoing.y == level.yexit && bonusl == NULL) {//if it is the exit and the exit is disponible
            freeFile(openFile);//free all the things that need to be freed
            freeFile(closedFile);
            freeMatrix((char **)tab_g, level.ysize);
            freeMatrix((char **)tab_f, level.ysize);
            freeMatrix((char **)heuristic, level.ysize);
            return retracePathForAStar(x, y, ongoing.x, ongoing.y, list_parents, level);//rettrrace path to get the right move
        }
        if (map[ongoing.y][ongoing.x] == BONUS) {//if the case is a bonus
            freeFile(openFile);//free all the things that need to be freed
            freeFile(closedFile);
            freeMatrix((char **)tab_g, level.ysize);
            freeMatrix((char **)tab_f, level.ysize);
            freeMatrix((char **)heuristic, level.ysize);
            return retracePathForAStar(x, y, ongoing.x, ongoing.y, list_parents, level);//rettrace path to get the right move
        }
        else {
            enfile(closedFile, ongoing);//else enfile the case in the closed file
            testNeighbors(map, ongoing.x, ongoing.y, tab_g, tab_f, list_parents, openFile, closedFile, heuristic);//and we check the neighbors
        }
    }
    freeFile(openFile);//free all the things that need to be freed
    freeFile(closedFile);
    freeMatrix((char **)tab_g, level.ysize);
    freeMatrix((char **)tab_f, level.ysize);
    freeMatrix((char **)heuristic, level.ysize);
    freeMatrix((char **)list_parents, level.ysize);
    return perduPourPerdu(map, x, y, level); //use the urgency function in the case where nothing else can be done
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////////////////////////

action lode_runner(levelinfo level, character_list characterl, bonus_list bonusl, bomb_list bombl) {
    //main function of the AI
    //add the bonus, the bombs and the ennemies on the map to make it easier to check where is what 
    //and not have to stock it in the memory each time you want to use bombl, bonusl or characterl
    //return the action choosen by the player

    action a;
    bool ok = false;
    int x, y;

    character_list pchar = characterl;
    do {
        if (pchar->c.item == RUNNER) {
            x = pchar->c.x; 
            y = pchar->c.y;
            ok = true;
        } else {
            pchar = pchar->next;
        }
    } while (!ok);

    bonus_list bonusl2 = bonusl;
    while (bonusl2 != NULL) { //put bonus on the map
        if (level.map[bonusl2->b.y][bonusl2->b.x] != BONUS) {
            level.map[bonusl2->b.y][bonusl2->b.x] = BONUS;
        }
        bonusl2 = bonusl2->next;
    }

    while (bombl != NULL) { //put bomb on the map
        if (level.map[bombl->y][bombl->x] != BOMB) {
            level.map[bombl->y][bombl->x] = BOMB;
        }
        bombl = bombl->next;
    }

    while (characterl != NULL) { //put ennemy on the map
        if (characterl->c.item == ENEMY) {
            level.map[characterl->c.y][characterl->c.x] = ENEMY;
        }
        characterl = characterl->next;
    }

    ok = false;
    do {
        a = aStarAlgorithm(level.map, x, y, bonusl, level);
        ok = true;

        if (DEBUG) {
            //printf("[Player] Candidate action ");
            print_action(a);
            //printf(ok ? " is valid.\n" : " not valid.\n");
        }
    } while (!ok);

    return a;
}

void print_action(action a) {
    switch (a) {
    case NONE: //printf("NONE"); break;
    case UP: //printf("UP"); break;
    case DOWN: //printf("DOWN"); break;
    case LEFT: //printf("LEFT"); break;
    case RIGHT: //printf("RIGHT"); break;
    case BOMB_LEFT: //printf("BOMB_LEFT"); break;
    case BOMB_RIGHT: //printf("BOMB_RIGHT"); break;
    }
}