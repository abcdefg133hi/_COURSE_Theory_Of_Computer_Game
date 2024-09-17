#include "solver.h"
#include "timer.h"
#include "step_table.h"
#include <stdio.h>
#include <string.h>
#include "board.h"
#include <stdlib.h>

#define HEAP_MAX 100000000

// Flag for if valid moved
#define _IF_VALID_MOVE_ALL 0
#define _IF_VALID_MOVE_DIAGONAL 1
#define _IF_VALID_MOVE_DIAGONAL_ONLY_CLOSE 2

int dice_seq[_dice_sequence_len_];
int goal_piece;
int min_step_to_move_goal[64][9][_dice_sequence_len_]; // piece_comb,distance,starting_point
int path[_dice_sequence_len_][2];
int time_depth = 100000000;

int max(int num1, int num2)
{
    if(num1>num2) return num1;
    else return num2;
}

void swap(int *num1, int *num2)
{
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

void swapMax(int *num1, int *num2)
{
    if(*num1>*num2) return;
    swap(num1, num2);
    return;
}
int digitDifference(int d1, int d2) // 0<=d<=9
{
    swapMax(&d1, &d2);
    return d1-d2;
}

struct LinkedList{
    struct LinkedList *parent;
    Board *board;

    int depth;
    int cost;
    int scores;
    int moved_dice;
    int start; int end;
 };

typedef struct LinkedList *Node;
Node finalNode;

Node newNode(Board *board, int cost, int scores, int depth, int moved_dice,
	       int start, int end,  Node parent)
{
    Node node = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    node->board = board;
    node->depth = depth;
    node->moved_dice = moved_dice;
    node->cost = cost;
    node->scores = scores;
    node->parent = parent;
    node->start = start;
    node->end = end;
    return node;
}
// Priority Queue


// Function to heapify the tree
struct Heap
{
    int size;
    int capacity;
    Node *array;
};

struct Heap *createHeap()
{
    struct Heap *newHeap = (struct Heap *)malloc(sizeof(struct Heap));

    newHeap->array = (Node *)malloc(sizeof(Node) * HEAP_MAX);
    newHeap->size = 0;
    newHeap->capacity = 1;
    return newHeap;
}

struct Heap* Insert(struct Heap* myHeap, Node node)
{
    myHeap->size++;
    int size = myHeap->size;
    myHeap->capacity++;

    myHeap->array[size] = node;
    int next = size>>1;
    int now = size;
    while(next!=0)
    {
	if(myHeap->array[now]->scores<=myHeap->array[next]->scores) break;
	else
	{
	    Node temp = myHeap->array[next];
	    myHeap->array[next] = myHeap->array[now];
	    myHeap->array[now] = temp;
	    now = now>>1;
	    next = next>>1;
	}
    }
    return myHeap;
}

Node Peep(struct Heap *myHeap){
    return myHeap->array[1];
}
struct Heap *Pop(struct Heap *myHeap)
{
    if(myHeap->size==0) return myHeap;
    Node node = myHeap->array[myHeap->size];
    int value = node->scores;
    myHeap->size--;
    int size = myHeap->size;
    myHeap->array[1] = node;
    int now = 1;
    while((now<<1)<=size)
    {
	if(myHeap->array[now<<1]->scores<=value && (now<<1)+1>size) break;
	else if(myHeap->array[now<<1]->scores>value && (now<<1)+1>size) //swap(now, now<<1)
	{
	    Node temp = myHeap->array[now<<1];
	    myHeap->array[now<<1] = myHeap->array[now];
	    myHeap->array[now] = temp;
	    break;
	}
	else if(myHeap->array[now<<1]->scores<value && myHeap->array[(now<<1)+1]->scores<value) break;
	else
	{
	    if(myHeap->array[now<<1]->scores>=value && myHeap->array[(now<<1)+1]->scores>=value)
	    {
		if(myHeap->array[now<<1]->scores>myHeap->array[(now<<1)+1]->scores)  //swap(now, now<<1)
		{
		    Node temp = myHeap->array[now<<1];
		    myHeap->array[now<<1] = myHeap->array[now];
		    myHeap->array[now] = temp;
		    now = now<<1;
		}
		else //swap(now, now<<1+1)
		{
		    Node temp = myHeap->array[(now<<1)+1];
		    myHeap->array[(now<<1)+1] = myHeap->array[now];
		    myHeap->array[now] = temp;
		    now = (now<<1)+1;
		}
	    }
	    else if(myHeap->array[now<<1]->scores<value) //swap(now, now<<1+1)
	    {
		Node temp = myHeap->array[(now<<1)+1];
		myHeap->array[(now<<1)+1] = myHeap->array[now];
		myHeap->array[now] = temp;
		now = (now<<1)+1;
	    }
	    else //swap(now, now<<1)
	    {
		Node temp = myHeap->array[now<<1];
		myHeap->array[now<<1] = myHeap->array[now];
		myHeap->array[now] = temp;
		now = now<<1;
	    }
	}
    }
    return myHeap;
}



int gen_moves(Board *board, int dice, int moves[16][2])
{
    int pce0 = dice2num[board->piece_bits][dice - 1][0];
    int pce1 = dice2num[board->piece_bits][dice - 1][1];
    int move_cnt = 0;
    int start0 = board->piece_position[pce0];
    for (int i = 0; i < 8; i++)
    {
        int endpos = step_table[start0][i];
        if (endpos == -1)
            break;
        moves[move_cnt][0] = pce0;
        moves[move_cnt][1] = endpos;
        move_cnt++;
    }
    if (pce1 == -1)
    {
        return move_cnt;
    }
    int start1 = board->piece_position[pce1];
    for (int i = 0; i < 8; i++)
    {
        int endpos = step_table[start1][i];
        if (endpos == -1)
            break;
        moves[move_cnt][0] = pce1;
        moves[move_cnt][1] = endpos;
        move_cnt++;
    }
    return move_cnt;
}

void move(Board *board, int moves[2])
{
    // check piece capture
    for (int i = 0; i < 6; i++)
    {
        if (board->piece_position[i] == moves[1])
        {
            board->piece_position[i] = -1;
            // set bit i to 0
            // its a tricky bit operation
            (board->piece_bits) &= (~(1 << i));
            break;
        }
    }
    board->piece_position[moves[0]] = moves[1];
}

// only used in sample code, you can delete it
#include <stdlib.h>
void random_walk(Board chessboard, int remain_depth, int search_depth)
{
    if (remain_depth == 0)
        return;
    // 8 directions and maximum 2 possible dice choices
    int moves[16][2];
    int move_num = gen_moves(&chessboard, dice_seq[search_depth], moves);
    int chosen_move = rand() % move_num;
    Board child = chessboard;
    move(&child, moves[chosen_move]);
    random_walk(child, remain_depth - 1, search_depth + 1);
    // back propagation (kind of)
    path[search_depth][0] = chessboard.piece_position[moves[chosen_move][0]];
    path[search_depth][1] = moves[chosen_move][1];
}

void CloneBoard(Board *new, Board *old)
{
    for(int i=0;i<6;i++)
    {
	new->piece_position[i] = old->piece_position[i];
    }
    new->piece_bits = old->piece_bits;
}

int if_valid_move(int moves[2], Board *chessboard, int flag)
{
    if(_IF_VALID_MOVE_ALL) return 1;
    if(_IF_VALID_MOVE_DIAGONAL)
    {
	if(moves[0]==goal_piece) return 1; // no restriction on goal piece
	int diff = chessboard->piece_position[moves[0]]- moves[1];
	if(diff==11 || diff==9 || diff==-11 || diff==-9) return 1;
	// Not diagonal
	for(int i=0;i<6;i++)
	{
	    if(i==goal_piece) continue; // Goal piece cannot be eaten
	    if(i==moves[0]) continue; // Itself
	    if(chessboard->piece_position[i] == moves[1]) return 1;
	}
	return 0;
    }
    if(_IF_VALID_MOVE_DIAGONAL_ONLY_CLOSE)
    {
	if(moves[0]==goal_piece) return 1; // no restriction on goal piece
	int diff = chessboard->piece_position[moves[0]]- moves[1];
	if(diff==11 || diff==9 || diff==-11 || diff==-9) return 1;
	// Not diagonal
	for(int i=0;i<6;i++)
	{
	    if(i==goal_piece) continue; // Goal piece cannot be eaten
	    if(i==moves[0]) continue; // Itself
	    if(chessboard->piece_position[i] == moves[1]) return 1;
	}
	return 0;
    }
}

void BFS_Visit(struct Heap *priority_queue)
{
    if (priority_queue->size == 0)
        return;
    Node current = Peep(priority_queue); priority_queue = Pop(priority_queue); // Retrieve the first element in PQ
    int search_depth = current->depth;
    Board *chessboard = current->board;
//printf("%d\n", search_depth);
//if(search_depth>2) return;
//printf("%d %d %d %d %d %d\n", chessboard->piece_position[0], chessboard->piece_position[1], chessboard->piece_position[2], chessboard->piece_position[3], chessboard->piece_position[4], chessboard->piece_position[5]);
//if(search_depth>2) return;
    if(chessboard->piece_position[goal_piece] == 0){
	finalNode = current;
	return;
    }
    // 8 directions and maximum 2 possible dice choices
    int moves[16][2];
    int move_num = gen_moves(chessboard, dice_seq[search_depth], moves);
    for(int i=0;i<move_num;i++){
	if(!if_valid_move(moves[i], chessboard, _IF_VALID_MOVE_DIAGONAL)) continue;
	Board *child = (Board *)malloc(sizeof(Board));
	CloneBoard(child, chessboard);
	move(child, moves[i]);
	Node node = newNode(child, 0, time_depth, search_depth+1, moves[i][0]+1,
		            chessboard->piece_position[moves[i][0]], child->piece_position[moves[i][0]], current);
	time_depth--;
	priority_queue = Insert(priority_queue, node);
    }
    BFS_Visit(priority_queue);
}

int Distance(int num1, int num2)
{
    int bit11 = num1 % 10;
    int bit12 = (num1 / 10) % 10;
    int bit21 = num2 % 10;
    int bit22 = (num2 / 10) % 10;
    int distance = max(digitDifference(bit11, bit21), digitDifference(bit12, bit22));
    return distance;
}


int ScoreForChess(Board *chessboard)
{
    int score = 0;
    for(int i=0;i<6;i++){
	if(chessboard->piece_position[i]!=-1) continue;
	if(goal_piece-i==1 || goal_piece-i==-1) score += 15;
	else score += 5;
    }
    return score;
}
void Heuristic_Search(struct Heap *priority_queue)
{
    if (priority_queue->size == 0)
        return;
    Node current = Peep(priority_queue); priority_queue = Pop(priority_queue); // Retrieve the first element in PQ
    int search_depth = current->depth;
    Board *chessboard = current->board;
//printf("%d\n", search_depth);
//printf("score: %d\n", current->scores);
//if(search_depth>2) return;
//printf("%d %d %d %d %d %d\n", chessboard->piece_position[0], chessboard->piece_position[1], chessboard->piece_position[2], chessboard->piece_position[3], chessboard->piece_position[4], chessboard->piece_position[5]);
//if(search_depth>5) return;
    if(chessboard->piece_position[goal_piece] == 0){
	finalNode = current;
	return;
    }
    // 8 directions and maximum 2 possible dice choices
    int moves[16][2];
    int move_num = gen_moves(chessboard, dice_seq[search_depth], moves);
    for(int i=0;i<move_num;i++){
	if(!if_valid_move(moves[i], chessboard, _IF_VALID_MOVE_DIAGONAL_ONLY_CLOSE)) continue;
	Board *child = (Board *)malloc(sizeof(Board));
	CloneBoard(child, chessboard);
	move(child, moves[i]);
	if(child->piece_position[goal_piece]==-1) continue;
	int goal_pos = child->piece_position[goal_piece];
	int score = 10000 - 30*Distance(goal_pos,0) - 30*(search_depth - 1) + ScoreForChess(child); // My constructed heap is max-heap. I am lazy....
    //printf("CH %d\n", child->piece_position[goal_piece]);
	Node node = newNode(child, 0, score, search_depth+1, moves[i][0]+1,
		            chessboard->piece_position[moves[i][0]], child->piece_position[moves[i][0]], current);
	priority_queue = Insert(priority_queue, node);
    }
    Heuristic_Search(priority_queue);
}
void solve(prob_layout problem_layout)
{
    // Start timer
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    // TODO: Write your searching algorithm here
    struct Heap *priority_queue = createHeap();





    // the sample code: random walk 20 steps

    // copy problem layout into board
    Board chessboard;
    chessboard.piece_bits = 0x3F;
    memcpy(chessboard.piece_position, problem_layout.piece_position, sizeof(problem_layout.piece_position));
    memcpy(dice_seq, problem_layout.dice_sequence, sizeof(dice_seq));

    // start from 0 <- start from 1
    goal_piece = problem_layout.goal_piece - 1;
    srand(time(NULL));

    int score = 10000 - Distance(chessboard.piece_position[goal_piece],0); // My constructed heap is max-heap. I am lazy....
    Node node = newNode(&chessboard, 0, score, 0, -1,
		        -1, -1,NULL);

    priority_queue = Insert(priority_queue, node);
    Heuristic_Search(priority_queue);
    //random_walk(chessboard, 20, 0);
    // print the result
    // endtimer
    // print step number
    int final_depth = finalNode->depth;
    //printf("20\n");
    /*
    for (int i = 0; i < 20; i++)
    {
        printf("%d %d ", path[i][0], path[i][1]);
    }
    */
    Node temp = finalNode;
    // Backpropogation
    while(temp->parent!=NULL){
	path[temp->depth-1][1] = temp->end;
	path[temp->depth-1][0] = temp->start;
	temp = temp->parent;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    double wall_clock_in_seconds = (double)((end.tv_sec+end.tv_nsec*1e-9)-(double)(start.tv_sec+start.tv_nsec*1e-9));
    printf("%lf\n",wall_clock_in_seconds);
    printf("%d\n", final_depth);
    for (int i = 0; i < final_depth; i++)
    {
        printf("%d %d ", path[i][0], path[i][1]);
    }
    printf("\n");
}
