#include "utils.h"
#include "message.h"
#include <stdbool.h>

void swap(int* x, int* y){
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

// Copies coord1 into coord2
void copy_coords(int n, bool* mr_alive, coordinate* coord1, coordinate* coord2){
    for (int i=0; i < n; i++)
        if (mr_alive[i])
            coord2[i] = coord1[i];
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
                    char* symbols, int* mr_arg1, int* mr_arg2, int* mr_arg3, int* mr_arg4){
    
    for (int i=0; i < no_of_monsters; i++){

        if (!mr_alive[i]) continue;

        int index = i;
        //coordinate min = {x_pos_of_mrs[i], y_pos_of_mrs[i]}; 
        //coordinate min = {mr_coords[i].x, mr_coords[i].y};
        coordinate min = mr_coords[i];

        for(int j=i; j < no_of_monsters; j++){

            if (!mr_alive[j]) continue;

            //if (x_pos_of_mrs[j] < min.x){
            if (mr_coords[j].x < min.x){
                //min.x = x_pos_of_mrs[j];
                //min.y = y_pos_of_mrs[j];
                
                min.x = mr_coords[j].x;
                min.y = mr_coords[j].y;
                
                index = j;
            }
            //else if (x_pos_of_mrs[j] == min.x && y_pos_of_mrs[j] < min.y){
            else if(mr_coords[j].x == min.x && mr_coords[j].y < min.y){
                //No need to change x
                //min.y = y_pos_of_mrs[j];
                min.y = mr_coords[j].y;
                index = j;
            }
        }
        // Time to locate the min. at the beginning

        if (i != index){
            swap(&mr_coords[i].x, &mr_coords[index].x);
            swap(&mr_coords[i].y, &mr_coords[index].y);
            
            swap(symbols+i, symbols+index);

            swap(mr_arg1+i, mr_arg1+index);
            swap(mr_arg2+i, mr_arg2+index);
            swap(mr_arg3+i, mr_arg3+index);
            swap(mr_arg4+i, mr_arg4+index);

            if (sort_pipes){
                swap(&mr_fds[i][0], &mr_fds[index][0]);
                swap(&mr_fds[i][1], &mr_fds[index][1]);
            }
        }
    }
}

int dist(coordinate coord1, coordinate coord2){
    return abs(coord1.x - coord2.x) + abs(coord1.y - coord2.y);
}