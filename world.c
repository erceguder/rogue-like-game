#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include "logging.h"
#include "message.h"
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

int main(void){

    int width_of_the_room, height_of_the_room;
    int x_pos_of_the_door, y_pos_of_the_door;
    int x_pos_of_the_player, y_pos_of_the_player;
    
    char exec_of_player[31];
    int player_arg1, player_arg2, player_arg3;

    int no_of_monsters;
    char exec_of_monsters[MONSTER_LIMIT][31], symbols[MONSTER_LIMIT]; // Not sure of path length...
    int monst_arg1[MONSTER_LIMIT], monst_arg2[MONSTER_LIMIT], monst_arg3[MONSTER_LIMIT], monst_arg4[MONSTER_LIMIT], monst_arg5[MONSTER_LIMIT], monst_arg6[MONSTER_LIMIT];

    int player_fd[2];
    //int* monster_fds[MONSTER_LIMIT] = {NULL}; // Needs garbage collection...

    scanf("%d %d", &width_of_the_room, &height_of_the_room);
    scanf("%d %d", &x_pos_of_the_door, &y_pos_of_the_door);
    scanf("%d %d", &x_pos_of_the_player, &y_pos_of_the_player);
    scanf("%s %d %d %d", exec_of_player, &player_arg1, &player_arg2, &player_arg3);
    scanf("%d", &no_of_monsters);
    
    int monster_fds[no_of_monsters][2];

    for(int i=0; i < no_of_monsters; i++)
        scanf("%s %c %d %d %d %d %d %d", exec_of_monsters[i], symbols+i, monst_arg1+i, monst_arg2+i, monst_arg3+i, monst_arg4+i, monst_arg5+i, monst_arg6+i);
    
    // Creating the pipe between the player and the game world.
    if(PIPE(player_fd) < 0)
        printf("Pipe for player could not be created\n");

    if (fork()){ // Game world goes here...
        close(player_fd[0]);
        //write(player_fd[1], "Erce\n", 10);
    }
    else{ // Player goes here...
        close(player_fd[1]);
        
        dup2(player_fd[0], 0);
        dup2(player_fd[0], 1);

        close(player_fd[0]);

        // Are you sure of 5 bytes?
        char x_pos_door_str[5], y_pos_door_str[5], player_arg1_str[5], player_arg2_str[5], player_arg3_str[5];

        sprintf(x_pos_door_str, "%d", x_pos_of_the_door);
        sprintf(y_pos_door_str, "%d", y_pos_of_the_door);
        sprintf(player_arg1_str, "%d", player_arg1);
        sprintf(player_arg2_str, "%d", player_arg2);
        sprintf(player_arg3_str, "%d", player_arg3);

        char* argv[] = {"player",  x_pos_door_str,  y_pos_door_str, player_arg1_str, player_arg2_str, player_arg3_str, NULL};
        execv("./player", argv);
    
        printf("This should not be printed...\n");
    }
    // Game world continues from here on...
    
    for (int i=0; i < no_of_monsters; i++){

        //monster_fds[i] = malloc(2*sizeof(int));

        // Creating a pipe between the player and a monster.
        if (PIPE(monster_fds[i]) < 0)
            printf("Pipe for monster %d could not be created\n", i);
        
        if (fork()){ // Game world goes here...
            close(monster_fds[i][0]);
        }
        else{ // Monster goes here...

            // Time to close the player FD.
            close(player_fd[1]);

            for(int j=1; j <= i; j++){ // Time to close FDs of monsters already created...
                close(monster_fds[i-j][1]);
            }

            close(monster_fds[i][1]);
            
            // At this point, monster has an FD of the pipe between the game world and the player... needs to be resolved. SOLVED.

            dup2(monster_fds[i][0], 0);
            dup2(monster_fds[i][0], 1);

            close(monster_fds[i][0]);
            
            // Are you sure of 5 bytes?
            char health[5], dmg_induced[5], defence[5], range[5];

            sprintf(health, "%d", monst_arg3[i]);
            sprintf(dmg_induced, "%d", monst_arg4[i]);
            sprintf(defence, "%d", monst_arg5[i]);
            sprintf(range, "%d", monst_arg6[i]);

            char* argv[] = {"monster", health, dmg_induced, defence, range, NULL};
            execv("./monster", argv);
        }
    }

    

    /*
    for (int i=0; i < no_of_monsters; i++) // Garbage collection
        free(monster_fds[i]);
    */
}