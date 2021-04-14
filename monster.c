#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "message.h"

// Pipe has been established... relax, kiddo

int main(int argc, char *argv[]){
    
    int health, dmg_induced, defence, range;

    health = atoi(argv[1]);
    dmg_induced = atoi(argv[2]);
    defence = atoi(argv[3]);
    range = atoi(argv[4]);

    printf("Monster -> health:%d, dmg_induced:%d, defence:%d, range:%d\n", health, dmg_induced, defence, range);

    monster_response response = {mr_ready, 0};
    write(1, response, sizeof(response));

    while (1){
        monster_message tmp;

        if (read(0, &tmp, sizeof(monster_message)) > 0){
            printf("Monster received some input\n");
        }
    }
}