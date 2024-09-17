long long next(long long input)
{
    if(input%2)
    {
        return 3*input+1;
    }
    return input/2;
}
int find_path_len_ratio(long long start)
{
    int len=0;
    long long current=start;
    while(current!=1)
    {
        current=next(current);
        len++;
    }
    int ret=1/(start/len);
    return ret;
}
int main()
{
    for(long long n=50;;n++)
    {
        find_path_len_ratio(n);
    }
}