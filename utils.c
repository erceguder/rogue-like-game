#include "utils.h"
#include "message.h"
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

void swap(int* x, int* y){
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void swap_chr(char* chr1, char* chr2){
    char tmp = *chr1;
    *chr1 = *chr2;
    *chr2 = tmp;
}

// Copies coord1 into coord2
void copy_coords(int n, bool* mr_alive, coordinate* coord1, coordinate* coord2){

    int skipped = 0;

    for (int i=0; i < n+skipped; i++){

        if (mr_alive[i])
            coord2[i-skipped] = coord1[i];
        else
            skipped++;
    }
}

bool occupied_by_monsters(int no_of_monsters, coordinate* mr_coords, bool* mr_alive, int x, int y){

    for (int i=0; i < no_of_monsters; i++){
                        
        if (mr_alive[i] && x == mr_coords[i].x && y == mr_coords[i].y){
            return true;
        }
    }
    return false;
}

void sort_monsters(bool sort_pipes, int mr_fds[][2], bool* mr_alive, int no_of_monsters, coordinate* mr_coords, 
                    char* symbols, int* mr_arg1, int* mr_arg2, int* mr_arg3, int* mr_arg4, char exec_of_monsters[][PATH_MAX]){
    
    for (int i=0; i < no_of_monsters; i++){

        //if (!mr_alive[i]) continue;

        int index = i;
        coordinate min = mr_coords[i];

        for(int j=i; j < no_of_monsters; j++){
            //if (!mr_alive[j]) continue;

            if (mr_coords[j].x < min.x){
                min = mr_coords[j];
                index = j;
            }
            else if(mr_coords[j].x == min.x && mr_coords[j].y < min.y){
                //No need to change x
                min.y = mr_coords[j].y;
                index = j;
            }
        }
        // Time to locate the min. at the beginning

        if (i != index){

            swap(&mr_coords[i].x, &mr_coords[index].x);
            swap(&mr_coords[i].y, &mr_coords[index].y);
            
            swap_chr(symbols+i, symbols+index);

            bool tmp = mr_alive[i];
            mr_alive[i] = mr_alive[index];
            mr_alive[index] = tmp;

            swap(mr_arg1+i, mr_arg1+index);
            swap(mr_arg2+i, mr_arg2+index);
            swap(mr_arg3+i, mr_arg3+index);
            swap(mr_arg4+i, mr_arg4+index);

            char tmp_str[PATH_MAX];
            strcpy(tmp_str, exec_of_monsters[i]);
            strcpy(exec_of_monsters[i], exec_of_monsters[index]);
            strcpy(exec_of_monsters[index], tmp_str);

            if (sort_pipes){
                //swap(&mr_fds[i][0], &mr_fds[index][0]); NOT NEEDED since they are already closed.
                swap(&mr_fds[i][1], &mr_fds[index][1]);
            }
        }
    }
}

int dist(coordinate coord1, coordinate coord2){
    return abs(coord1.x - coord2.x) + abs(coord1.y - coord2.y);
}