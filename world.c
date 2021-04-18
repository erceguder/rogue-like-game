#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "message.h"
#include "utils.h"

#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

int main(void){

    int width_of_the_room, height_of_the_room;
    int x_pos_of_the_door, y_pos_of_the_door;
    
    coordinate pr_coord;
    coordinate mr_coords[MONSTER_LIMIT];

    char exec_of_player[PATH_MAX];
    //int player_arg1, player_arg2, player_arg3;
    char player_arg1[1024], player_arg2[1024], player_arg3[1024];

    int no_of_monsters, tmp_no_of_monsters;
    char path_of_monsters[MONSTER_LIMIT][PATH_MAX], symbols[MONSTER_LIMIT];
    //int mr_arg1[MONSTER_LIMIT], mr_arg2[MONSTER_LIMIT], mr_arg3[MONSTER_LIMIT], mr_arg4[MONSTER_LIMIT];
    char mr_arg1[MONSTER_LIMIT][1024], mr_arg2[MONSTER_LIMIT][1024], mr_arg3[MONSTER_LIMIT][1024], mr_arg4[MONSTER_LIMIT][1024];

    int player_fd[2];
    int monster_fds[MONSTER_LIMIT][2];

    game_over_status go_status;

    bool mr_alive[MONSTER_LIMIT];

    for (int i=0; i < MONSTER_LIMIT; i++)
        mr_alive[i] = true;

    scanf("%d %d", &width_of_the_room, &height_of_the_room);
    scanf("%d %d", &x_pos_of_the_door, &y_pos_of_the_door);
    scanf("%d %d", &pr_coord.x, &pr_coord.y);
    //scanf("%s %d %d %d", exec_of_player, &player_arg1, &player_arg2, &player_arg3);
    scanf("%s %s %s %s", exec_of_player, player_arg1, player_arg2, player_arg3);
    scanf("%d", &no_of_monsters);

    for(int i=0; i < no_of_monsters; i++)
        //scanf("%s %c %d %d %d %d %d %d", path_of_monsters[i], symbols+i, &mr_coords[i].x, &mr_coords[i].y, mr_arg1+i, mr_arg2+i, mr_arg3+i, mr_arg4+i);
        scanf("%s %c %d %d %s %s %s %s", path_of_monsters[i], symbols+i, &mr_coords[i].x, &mr_coords[i].y, mr_arg1[i], mr_arg2[i], mr_arg3[i], mr_arg4[i]);

    // Ordering of monsters
    // TODO: CHANGE THE ORDER OF EXECUTABLE PATHS ! Done.
    
    sort_monsters(false, monster_fds, mr_alive, no_of_monsters, mr_coords, symbols, mr_arg1, mr_arg2, mr_arg3, mr_arg4, path_of_monsters);

    /*
    for (int i=0; i < no_of_monsters; i++){
        printf("Executable path of %d is %s\n", i, path_of_monsters[i]);
        printf("Symbol: %c\n", symbols[i]);
        printf("x:%d, y:%d\n", mr_coords[i].x, mr_coords[i].y);
        printf("Arg.1 is %s\n", mr_arg1[i]);
        printf("Arg.2 is %s\n", mr_arg2[i]);
        printf("Arg.3 is %s\n", mr_arg3[i]);
        printf("Arg.4 is %s\n", mr_arg4[i]);
    }
    */

    // Creating the pipe between the player and the game world.
    if(PIPE(player_fd) < 0)
        exit(1);

    if (fork()){ // Game world goes here...
        close(player_fd[0]);
    }
    else{ // Player goes here...
        close(player_fd[1]);
        
        dup2(player_fd[0], 0);
        dup2(player_fd[0], 1);

        close(player_fd[0]);

        // Are you sure of 15 bytes?
        //char x_pos_door_str[15], y_pos_door_str[15], player_arg1_str[15], player_arg2_str[15], player_arg3_str[15];

        char x_pos_door_str[15], y_pos_door_str[15];

        sprintf(x_pos_door_str, "%d", x_pos_of_the_door);
        sprintf(y_pos_door_str, "%d", y_pos_of_the_door);
        /*
        sprintf(player_arg1_str, "%d", player_arg1);
        sprintf(player_arg2_str, "%d", player_arg2);
        sprintf(player_arg3_str, "%d", player_arg3);
        */

        //char* argv[] = {exec_of_player,  x_pos_door_str,  y_pos_door_str, player_arg1_str, player_arg2_str, player_arg3_str, NULL};
        char* argv[] = {exec_of_player,  x_pos_door_str,  y_pos_door_str, player_arg1, player_arg2, player_arg3, NULL};
        execv(exec_of_player, argv);
    
        printf("This should not be printed 1...\n");
    }
    // Game world continues from here on...
    
    for (int i=0; i < no_of_monsters; i++){

        // Creating a pipe between the player and a monster.
        if (PIPE(monster_fds[i]) < 0)
            exit(1);

        if (fork()){ // Game world goes here...
            close(monster_fds[i][0]);
        }
        else{ // Monster goes here...

            // At this point, monster has an FD of the pipe between the game world and the player... needs to be resolved. SOLVED.
            // Time to close the player FD.

            close(player_fd[1]);

            for(int j=1; j <= i; j++){ // Time to close FDs of monsters already created...
                close(monster_fds[i-j][1]);
            }

            close(monster_fds[i][1]);

            dup2(monster_fds[i][0], 0);
            dup2(monster_fds[i][0], 1);

            close(monster_fds[i][0]);

            // Are you sure of 15 bytes?
            /*
            char health[15], dmg_induced[15], defence[15], range[15];

            sprintf(health, "%d", mr_arg1[i]);
            sprintf(dmg_induced, "%d", mr_arg2[i]);
            sprintf(defence, "%d", mr_arg3[i]);
            sprintf(range, "%d", mr_arg4[i]);
            */

            //char* argv[] = {path_of_monsters[i], health, dmg_induced, defence, range, NULL};
            char* argv[] = {path_of_monsters[i], mr_arg1[i], mr_arg2[i], mr_arg3[i], mr_arg4[i]};
            execv(path_of_monsters[i], argv);

            printf( "This should not be printed 2...\n");
        }
    }
    
    player_response pr_response;
    monster_response mr_response;

    // Waiting for children to be "ready".
    {
        read(player_fd[1], &pr_response, sizeof(player_response));

        // NOT SURE OF THIS LOOP.
        while (pr_response.pr_type != pr_ready)
            read(player_fd[1], &pr_response, sizeof(player_response));
        
        for (int i=0; i < no_of_monsters; i++){
            read(monster_fds[i][1], &mr_response, sizeof(monster_response));

            // NOT SURE OF THIS LOOP.
            while(mr_response.mr_type != mr_ready)
                read(monster_fds[i][1], &mr_response, sizeof(monster_response));
        }
    }
    // Printing initial map
    {
        map_info map;

        coordinate door_coord = {x_pos_of_the_door, y_pos_of_the_door};
        

        for(int i=0; i < no_of_monsters; i++){

            map.monster_coordinates[i].x = mr_coords[i].x;
            map.monster_coordinates[i].y = mr_coords[i].y;

            map.monster_types[i] = symbols[i];
        }

        map.map_width = width_of_the_room; map.map_height = height_of_the_room;
        map.alive_monster_count = no_of_monsters;
        map.door = door_coord;
        map.player = pr_coord;

        print_map(&map);
    }

    // Main loop goes here...

    int damage_inflicted_on_player = 0;
    int damage_inflicted_on_mrs[MONSTER_LIMIT] = {0};

    bool game_over = false;

    tmp_no_of_monsters = no_of_monsters;

    while (!game_over){

        // Clear damage inflicted on monsters
        for (int i=0; i < MONSTER_LIMIT; i++)
            damage_inflicted_on_mrs[i] = 0;

        player_message pr_msg;
        monster_message mr_msg;

        pr_msg.new_position = pr_coord;
        pr_msg.total_damage = damage_inflicted_on_player;
        pr_msg.alive_monster_count = tmp_no_of_monsters;

        // This copies dead monster coordinates... SOLVED.
        copy_coords(tmp_no_of_monsters, mr_alive, mr_coords, pr_msg.monster_coordinates);

        pr_msg.game_over = game_over;

        write(player_fd[1], &pr_msg, sizeof(pr_msg));

        bool player_left = !(read(player_fd[1], &pr_response, sizeof(player_response)));

        if (player_left){
            go_status = go_left;
            game_over = true;

            int* child_status;
            wait(child_status);
        }

        switch(pr_response.pr_type){
            
            case pr_move:
                ; // This is needed to avoid silly errors...
                coordinate new_pos = pr_response.pr_content.move_to;

                //Player is trying to move into a wall OR A DOOR or further away...
                if (new_pos.x <= 0 || new_pos.x >= width_of_the_room - 1 ||
                    new_pos.y <= 0 || new_pos.y >= height_of_the_room - 1){
                        
                        if (new_pos.x == x_pos_of_the_door && new_pos.y == y_pos_of_the_door){
                            
                            go_status = go_reached;
                            game_over = true;

                            pr_coord.x = new_pos.x;
                            pr_coord.y = new_pos.y;

                            //Inform player process, and reap it
                            pr_msg.game_over = true;
                            write(player_fd[1], &pr_msg, sizeof(pr_msg));

                            int* child_status;
                            wait(child_status);
                        }
                        
                        // else: Nothing to do, as player cannot move into a wall...
                }
                else{ // Check if another entity occupies the coordinate

                    // Update player position
                    if (!occupied_by_monsters(no_of_monsters, mr_coords, mr_alive, new_pos.x, new_pos.y)){

                        pr_coord.x = new_pos.x;
                        pr_coord.y = new_pos.y;
                    }
                }
                break;

            case pr_attack:

                for (int i=0; i < no_of_monsters; i++){

                    if(pr_response.pr_content.attacked[i] > 0){

                        // Sync'ing the array sent to the player and game_world's...

                        int alive_cnt = 0;
                        int index;

                        for (index=0; alive_cnt <= i; index++){
                            if (mr_alive[index])
                                alive_cnt++;
                        }
                        index--; // Needs trimming

                        damage_inflicted_on_mrs[index] = pr_response.pr_content.attacked[i];
                    }
                }

                break;
            
            case pr_dead:

                go_status = go_died;
                game_over = true;

                int* child_status;
                wait(child_status);

                break;
        }

        damage_inflicted_on_player = 0;

        for (int i=0; i < no_of_monsters; i++){

            if (mr_alive[i] == false) continue;

            mr_msg.new_position = mr_coords[i];
            mr_msg.damage = damage_inflicted_on_mrs[i];
            
            mr_msg.player_coordinate = pr_coord;
            mr_msg.game_over = game_over;

            write(monster_fds[i][1], &mr_msg, sizeof(mr_msg));

            // After sending game_over signal, why is this not waiting? EOF.
            read(monster_fds[i][1], &mr_response, sizeof(monster_response));

            switch(mr_response.mr_type){
                case mr_move:
                    ; // This is needed to avoid silly errors...

                    coordinate new_pos = mr_response.mr_content.move_to;
                    
                    if (new_pos.x > 0 && new_pos.x < width_of_the_room - 1 &&
                        new_pos.y > 0 && new_pos.y < height_of_the_room -1){

                        bool occupied = occupied_by_monsters(no_of_monsters, mr_coords, mr_alive, new_pos.x, new_pos.y) ||
                                                            (new_pos.x == pr_coord.x && new_pos.y == pr_coord.y);

                        if (!occupied){

                            mr_coords[i].x = new_pos.x;
                            mr_coords[i].y = new_pos.y;
                        }
                    }
                    break;

                case mr_attack:

                    damage_inflicted_on_player += mr_response.mr_content.attack;
                    break;

                case mr_dead:

                    mr_alive[i] = false;

                    int* child_status;
                    wait(child_status);

                    tmp_no_of_monsters--;
                    
                    break;
            }
        }

        // Re-ordering of monsters go here. NOTE: Keep pipes in mind.
        sort_monsters(true, monster_fds, mr_alive, no_of_monsters, mr_coords, symbols, mr_arg1, mr_arg2, mr_arg3, mr_arg4, path_of_monsters);
        
        if (tmp_no_of_monsters == 0){
            game_over = true;
            go_status = go_survived;

            pr_msg.game_over = true;
            write(player_fd[1], &pr_msg, sizeof(pr_msg));

            int* child_status;
            wait(child_status);
        }

        // Printing updated map
        {
            map_info map;

            coordinate door_coord = {x_pos_of_the_door, y_pos_of_the_door};

            int skipped = 0;

            for(int i=0; i < no_of_monsters; i++){
                if (mr_alive[i]){

                    map.monster_coordinates[i-skipped] = mr_coords[i];
                    map.monster_types[i-skipped] = symbols[i];
                }
                else
                    skipped++;
            }

            map.map_width = width_of_the_room; map.map_height = height_of_the_room;
            map.alive_monster_count = tmp_no_of_monsters;
            map.door = door_coord;
            map.player = pr_coord;

            print_map(&map);
        }
    }

    int* child_status;
    
    for(int i=0; i < tmp_no_of_monsters; i++){
        wait(child_status);
    }

    print_game_over(go_status);

    // Closing all the FD's left open

    close(player_fd[1]);
    for (int i=0; i < no_of_monsters; i++)
        close(monster_fds[i][1]);
    
    //while(1); Used to test reaping
}