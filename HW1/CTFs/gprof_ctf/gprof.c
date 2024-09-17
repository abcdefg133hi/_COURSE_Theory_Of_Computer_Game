#include<stdio.h>
#define GOAL_LENGTH 500
long long next(long long input)
{
    if(input%2)
    {
        return 3*input+1;
    }
    return input/2;
}
int find_path_len(long long start)
{
    int len=0;
    long long current=start;
    while(current!=1)
    {
        current=next(current);
        len++;
    }
    return len;
}
int main()
{
    for(long long n=5;;n++)
    {
        int len=find_path_len(n);
        if(len>GOAL_LENGTH)
        {
            printf("The smallest N that takes more than %d steps to reach 4->2->1 loop is %lld\n",GOAL_LENGTH,n);
            printf("Which length is %d\n",len);
            break;
        }
    }
}