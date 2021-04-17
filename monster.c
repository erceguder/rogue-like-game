#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "message.h"
#define MAX(a,b) (((a)>(b))?(a):(b))

int dist(coordinate coord1, coordinate coord2){
    return abs(coord1.x - coord2.x) + abs(coord1.y - coord2.y);
}

// Pipe has been established... relax, kiddo
// Printf does NOT work!!!

int main(int argc, char *argv[]){
    int health, attack_dmg, defence, range;

    health = atoi(argv[1]);
    attack_dmg = atoi(argv[2]);
    defence = atoi(argv[3]);
    range = atoi(argv[4]);

    monster_response_content mr_content;// = {0};
    
    memset(&mr_content, 0, sizeof(monster_response_content));

    monster_response response = {mr_ready, mr_content};
    write(1, &response, sizeof(response));

    while (1){
        monster_message msg;

        if (read(0, &msg, sizeof(monster_message)) > 0){
            
            if (msg.game_over)
                exit(0);

            health = health - MAX(0, msg.damage - defence);
            //defence = MAX(0, defence - msg.damage);

            if (health <= 0){ // Monster's dead...
                
                response.mr_type = mr_dead;
                memset(&mr_content, 0, sizeof(monster_response_content));
                response.mr_content = mr_content;

                write(1, &response, sizeof(response));
                exit(0);
            }

            // Player is in the range of the monster
            if (dist(msg.new_position, msg.player_coordinate) <= range){

                response.mr_type = mr_attack;
                mr_content.attack = attack_dmg;
                response.mr_content = mr_content;

                write(1, &response, sizeof(response));
            }
            else{ // Monster should move closer to the player

                coordinate tmp, min_cell;
                int min_dist;

                // Upper cell
                tmp.x = msg.new_position.x;
                tmp.y = msg.new_position.y - 1;
                min_dist = dist(tmp, msg.player_coordinate);
                min_cell = tmp;

                // Upper-right cell
                tmp.x = msg.new_position.x+1;
                tmp.y = msg.new_position.y-1;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Right cell
                tmp.x = msg.new_position.x+1;
                tmp.y = msg.new_position.y;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Below-right cell
                tmp.x = msg.new_position.x+1;
                tmp.y = msg.new_position.y+1;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Below cell
                tmp.x = msg.new_position.x;
                tmp.y = msg.new_position.y+1;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Below-left cell
                tmp.x = msg.new_position.x-1;
                tmp.y = msg.new_position.y+1;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Left cell
                tmp.x = msg.new_position.x-1;
                tmp.y = msg.new_position.y;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                // Upper-left cell
                tmp.x = msg.new_position.x-1;
                tmp.y = msg.new_position.y-1;
                
                if (dist(tmp, msg.player_coordinate) < min_dist){
                    min_dist = dist(tmp, msg.player_coordinate);
                    min_cell = tmp;
                }

                response.mr_type = mr_move;
                mr_content.move_to = min_cell;
                response.mr_content = mr_content;

                write(1, &response, sizeof(response));
            }
        }
    }
}