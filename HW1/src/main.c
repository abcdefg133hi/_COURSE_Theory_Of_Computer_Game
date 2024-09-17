#include<stdio.h>
#include"lib/input_parser.h"
#include"code/solver.h"
int main()
{
    // parse input into parsed_layout
    prob_layout parsed_layout=parse_input();
    // TODO: modify the solve in src/code/solver.c to solve the problem
    solve(parsed_layout);
    return 0;
}