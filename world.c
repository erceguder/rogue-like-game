#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include "logging.c"
#include "message.h"
#include <stdbool.h>

#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

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

int main(void){

    int width_of_the_room, height_of_the_room;
    int x_pos_of_the_door, y_pos_of_the_door;
    int x_pos_of_the_player, y_pos_of_the_player;
    
    char exec_of_player[31]; // Not sure of path length...
    int player_arg1, player_arg2, player_arg3;

    int no_of_monsters, tmp_no_of_monsters;
    char exec_of_monsters[MONSTER_LIMIT][31], symbols[MONSTER_LIMIT]; // Not sure of path length...
    int x_pos_of_mrs[MONSTER_LIMIT], y_pos_of_mrs[MONSTER_LIMIT], mr_arg1[MONSTER_LIMIT], mr_arg2[MONSTER_LIMIT], mr_arg3[MONSTER_LIMIT], mr_arg4[MONSTER_LIMIT];

    int player_fd[2];

    game_over_status go_status;

    scanf("%d %d", &width_of_the_room, &height_of_the_room);
    scanf("%d %d", &x_pos_of_the_door, &y_pos_of_the_door);
    scanf("%d %d", &x_pos_of_the_player, &y_pos_of_the_player);
    scanf("%s %d %d %d", exec_of_player, &player_arg1, &player_arg2, &player_arg3);
    scanf("%d", &no_of_monsters);
    
    int monster_fds[MONSTER_LIMIT][2];

    coordinate pr_coord = {x_pos_of_the_player, y_pos_of_the_player};
    coordinate mr_coords[MONSTER_LIMIT];

    bool mr_alive[20]; // Variable size not allowed ??????? Did it above.

    for (int i=0; i < MONSTER_LIMIT; i++)
        mr_alive[i] = true;

    //for (int i=0; i < MONSTER_LIMIT; i++)
      //  printf("mr_alive[%d]=%d\n", i, mr_alive[i]);

    for(int i=0; i < no_of_monsters; i++)
        scanf("%s %c %d %d %d %d %d %d", exec_of_monsters[i], symbols+i, x_pos_of_mrs+i, y_pos_of_mrs+i, mr_arg1+i, mr_arg2+i, mr_arg3+i, mr_arg4+i);
    
    // Ordering of monsters
    // ORDER EXECUTABLE PATHS !
    {
        for (int i=0; i < no_of_monsters; i++){

            int index = i;
            coordinate min = {x_pos_of_mrs[i], y_pos_of_mrs[i]};

            for(int j=i; j < no_of_monsters; j++){

                if (x_pos_of_mrs[j] < min.x){
                    min.x = x_pos_of_mrs[j];
                    min.y = y_pos_of_mrs[j];
                    index = j;
                }
                else if (x_pos_of_mrs[j] == min.x && y_pos_of_mrs[j] < min.y){
                    //No need to change x
                    min.y = y_pos_of_mrs[j];
                    index = j;
                }
            }
            // Time to locate the min. at the beginning
            swap(x_pos_of_mrs+i, x_pos_of_mrs+index);
            swap(y_pos_of_mrs+i, y_pos_of_mrs+index);
            swap(symbols+i, symbols+index);

            swap(mr_arg1+i, mr_arg1+index);
            swap(mr_arg2+i, mr_arg2+index);
            swap(mr_arg3+i, mr_arg3+index);
            swap(mr_arg4+i, mr_arg4+index);
        }
    }
    // Filling mr_coords
    {
        for (int i=0; i < no_of_monsters; i++){
            mr_coords[i].x = x_pos_of_mrs[i];
            mr_coords[i].y = y_pos_of_mrs[i];
        }
    }

    // Creating the pipe between the player and the game world.
    if(PIPE(player_fd) < 0)
        printf("Pipe for player could not be created\n");

    if (fork()){ // Game world goes here...
        close(player_fd[0]);
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
    
        //printf("This should not be printed...\n"); Is this going to work, really? After overwriting stdout?
        printf("This should not be printed 1...\n");
    }
    // Game world continues from here on...
    
    for (int i=0; i < no_of_monsters; i++){

        // Creating a pipe between the player and a monster.
        if (PIPE(monster_fds[i]) < 0)
            printf( "Pipe for monster %d could not be created\n", i);

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

            // Are you sure of 5 bytes?
            char health[5], dmg_induced[5], defence[5], range[5];

            sprintf(health, "%d", mr_arg1[i]);
            sprintf(dmg_induced, "%d", mr_arg2[i]);
            sprintf(defence, "%d", mr_arg3[i]);
            sprintf(range, "%d", mr_arg4[i]);

            char* argv[] = {"monster", health, dmg_induced, defence, range, NULL};
            execv("./monster", argv);

            //printf("This should not be printed...\n"); //Is this going to work, really? After overwriting stdout?
            printf( "This should not be printed 2...\n");
        }
    }
    
    player_response pr_response;
    monster_response mr_response;

    read(player_fd[1], &pr_response, sizeof(player_response));

    // Waiting for player response to be "ready".
    // NOT SURE OF THIS LOOP.
    while (pr_response.pr_type != pr_ready)
        read(player_fd[1], &pr_response, sizeof(player_response));
    
    //printf( "Received ready from player...\n");
    
    // Waiting for monsters to be "ready".
    for (int i=0; i < no_of_monsters; i++){
        read(monster_fds[i][1], &mr_response, sizeof(monster_response));

        // NOT SURE OF THIS LOOP.
        while(mr_response.mr_type != mr_ready)
            read(monster_fds[i][1], &mr_response, sizeof(monster_response));

        //printf( "Monster %d is ready...\n", i);
    }

    // Printing initial map
    {
        map_info map;

        coordinate door_coord = {x_pos_of_the_door, y_pos_of_the_door};
        coordinate player_coord = {x_pos_of_the_player, y_pos_of_the_player};

        for(int i=0; i < no_of_monsters; i++){
            coordinate tmp = {x_pos_of_mrs[i], y_pos_of_mrs[i]};
            map.monster_coordinates[i] = tmp;
            map.monster_types[i] = symbols[i];
        }

        map.map_width = width_of_the_room; map.map_height = height_of_the_room;
        map.alive_monster_count = no_of_monsters;
        map.door = door_coord; map.player = player_coord;

        /*
        for (int i=0; i < no_of_monsters; i++){
            printf("%dth monster's x:%d, ", i, map.monster_coordinates[i].x);
            printf("y:%d, ", map.monster_coordinates[i].y);
            printf("symbol: %c\n", map.monster_types[i]);
        }
        */

       print_map(&map);
    }

    // Main loop goes here...

    int damage_inflicted_on_player = 0;
    int damage_inflicted_on_mrs[MONSTER_LIMIT] = {0}; // Variable size not allowed ??????? Did it above.

    bool game_over = false;

    tmp_no_of_monsters = no_of_monsters;

    while (!game_over){

        player_message pr_msg;
        monster_message mr_msg;

        pr_msg.new_position = pr_coord;
        pr_msg.total_damage = damage_inflicted_on_player;
        pr_msg.alive_monster_count = tmp_no_of_monsters;
        //pr_msg.monster_coordinates = mr_coords;

        // This copies dead monster coordinates... SOLVED.
        copy_coords(no_of_monsters, mr_alive, mr_coords, pr_msg.monster_coordinates);

        pr_msg.game_over = game_over;

        write(player_fd[1], &pr_msg, sizeof(pr_msg));

        read(player_fd[1], &pr_response, sizeof(player_response));

        switch(pr_response.pr_type){
            
            case pr_move:
                
                //printf("Player wants to move\n");

                ; // This is needed to avoid silly errors...
                coordinate new_pos = pr_response.pr_content.move_to;

                //Player is trying to move into a wall OR A DOOR or further away...
                if (new_pos.x <= 0 || new_pos.x >= width_of_the_room - 1 ||
                    new_pos.y <= 0 || new_pos.y >= height_of_the_room - 1){
                        
                        if (new_pos.x == x_pos_of_the_door && new_pos.y == y_pos_of_the_door){
                            // go_reached
                            go_status = go_reached;
                            game_over = true;
                        }
                        
                        // else: Nothing to do, as player cannot move into a wall...
                }
                else{ // Check if another entity occupies the coordinate

                    // Update player position
                    if (!occupied_by_monsters(no_of_monsters, mr_coords, mr_alive, new_pos.x, new_pos.y)){

                        //printf("Player moved to (%d, %d)\n", new_pos.x, new_pos.y);

                        pr_coord.x = new_pos.x;
                        pr_coord.y = new_pos.y;
                    }
                }
                break;

            case pr_attack:
                
                //printf("Player wants to attac\n");

                for (int i=0; i < no_of_monsters; i++){

                    if(mr_alive[i] && pr_response.pr_content.attacked[i] > 0){

                        //printf("Player attacked monster %d\n", i);

                        damage_inflicted_on_mrs[i] = pr_response.pr_content.attacked[i];
                    }
                }

                break;
            
            case pr_dead:

                //printf("Player is dead :(\n");

                go_status = go_died;
                game_over = true;
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

            read(monster_fds[i][1], &mr_response, sizeof(monster_response));

            switch(mr_response.mr_type){

                case mr_move:
                    ; // This is needed to avoid silly errors...

                    //printf("Monster %d wants to move\n", i);

                    coordinate new_pos = mr_response.mr_content.move_to;
                    
                    if (new_pos.x > 0 && new_pos.x < width_of_the_room - 1 &&
                        new_pos.y > 0 && new_pos.y < height_of_the_room -1){

                        bool occupied = occupied_by_monsters(no_of_monsters, mr_coords, mr_alive, new_pos.x, new_pos.y) ||
                                    (new_pos.x == pr_coord.x && new_pos.y == pr_coord.y);

                        if (!occupied){

                            //printf("Monster %d moved to (%d, %d)\n", i, new_pos.x, new_pos.y);

                            mr_coords[i].x = new_pos.x;
                            mr_coords[i].y = new_pos.y;
                        }
                    }
                    break;

                case mr_attack:

                    //printf("Monster %d wants to attac\n", i);

                    damage_inflicted_on_player += mr_response.mr_content.attack;
                    break;

                case mr_dead:

                    //printf("Monster %d is dead :)\n", i);

                    mr_alive[i] = false;

                    int* child_status;
                    wait(child_status);

                    tmp_no_of_monsters--;
                    
                    break;
            }
        }

        if (tmp_no_of_monsters == 0){
            game_over = true;
            go_status = go_survived;
        }

        // Printing updated map
        {
            map_info map;

            coordinate door_coord = {x_pos_of_the_door, y_pos_of_the_door};

            for(int i=0; i < no_of_monsters; i++){
                if (mr_alive[i]){
                    //printf("Printing stage: monster %d is alive.\n", i);
                    map.monster_coordinates[i] = mr_coords[i];
                    map.monster_types[i] = symbols[i];
                }
            }

            map.map_width = width_of_the_room; map.map_height = height_of_the_room;
            map.alive_monster_count = tmp_no_of_monsters;
            map.door = door_coord; map.player = pr_coord;

            print_map(&map);
        }
    }

    int* child_status;
    
    for(int i=0; i < tmp_no_of_monsters; i++){
        wait(child_status);
    }

    print_game_over(go_status);

    //printf( "\nReturning from main of world...\n");
}