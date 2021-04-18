#ifndef UTILS_H
#define UTILS_H

#define MAX(a,b) (((a)>(b))?(a):(b))

#include <limits.h>
#include "message.h"

void swap(int* x, int* y);

void copy_coords(int n, bool* mr_alive, coordinate* coord1, coordinate* coord2);

bool occupied_by_monsters(int no_of_monsters, coordinate* mr_coords, bool* mr_alive, int x, int y);

void sort_monsters(bool sort_pipes, int mr_fds[][2], bool* mr_alive, int no_of_monsters, coordinate* mr_coords, 
                    char* symbols, char mr_arg1[][1024], char mr_arg2[][1024], char mr_arg3[][1024], char mr_arg4[][1024], char exec_of_monsters[][PATH_MAX]);

int dist(coordinate coord1, coordinate coord2);
#endif