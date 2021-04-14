#include "logging.h"

#include <stdio.h>

void print_map(map_info *mi) {
  int mc = 0;
  for (int y = 0; y < mi->map_height; y++) {
    for (int x = 0; x < mi->map_width; x++) {
      if (x == mi->player.x && y == mi->player.y) {
        printf("@");
      } else if (mc < mi->alive_monster_count &&
                 x == mi->monster_coordinates[mc].x &&
                 y == mi->monster_coordinates[mc].y) {
        printf("%c", mi->monster_types[mc]);
        mc++;
      } else if (x == mi->door.x && y == mi->door.y) {
        printf("+");
      } else if (x == 0 || x == mi->map_width - 1 || y == 0 ||
                 y == mi->map_height - 1) {
        printf("#");
      } else {
        printf(".");
      }
    }
    printf("\n");
  }
  printf("\n");
}

void print_game_over(game_over_status go) {
  printf("Game Over\nPlayer ");
  switch (go) {
  case go_reached:
    printf("won by reaching the door\n");
    break;
  case go_survived:
    printf("won by surviving all monsters\n");
    break;
  case go_left:
    printf("lost by leaving the game\n");
    break;
  case go_died:
    printf("lost by dying\n");
    break;
  }
}
