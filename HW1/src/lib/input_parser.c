#include "input_parser.h"
#include <stdio.h>
prob_layout parse_input()
{
    prob_layout parsed_layout;
    scanf("%d",&parsed_layout.goal_piece);
    for(int i=0;i<6;i++)
    {
        scanf("%d",&parsed_layout.piece_position[i]);
    }
    getchar();//skip the return char '\n'
    for(int i=0;i<_dice_sequence_len_;i++)
    {
        parsed_layout.dice_sequence[i]=getchar()-'0';
    }
    return parsed_layout;
}