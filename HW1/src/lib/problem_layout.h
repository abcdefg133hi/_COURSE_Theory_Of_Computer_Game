#ifndef __prob_layout__
#define __prob_layout__ 1
#define _dice_sequence_len_ 30
typedef struct problayout{
    int goal_piece;
    int piece_position[6];
    int dice_sequence[_dice_sequence_len_];
}prob_layout;
#endif
