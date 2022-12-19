#include "api_req_async.hpp"

int isSubString(StringType &dest, char end_of_data[])
{
    long long i = dest.length;
    // printf("i->%ld,%d,%s\n",i,i>=0,dest.ch);
    if(i<=0){
        return -1;
    }
    long long end_len = strlen(end_of_data);
    while (i-- >= 0)
    {
        // printf("i->%d,%d\n",i,i>=0);
        if (dest.ch[i] == end_of_data[end_len - 1])
        {
            // printf("i->%d,%c\n",i,dest.ch[i]);
            for (long long j = end_len - 1; j >= 0 && i >= 0; j--)
            {
                // printf("i->%d,j->%d,%c,%c\n",i,j,dest.ch[i],end_of_data[j]);
                if (dest.ch[i--] == end_of_data[j])
                {                    
                    if (j == 0)
                        return i + end_len + 1;
                }
                else
                {
                    break;
                }
            }
        }
    }
    return -1;
}

void my_strcpy(StringType &dest, char *src, long long length)
{
    if (length <= 0)
        return;
    long long prev_length = dest.length;
    // char temp_dest[prev_length];
   
    char *temp_dest=(char *)malloc(sizeof(char) * (prev_length + 1));
    // resize & repopulate
    memmove(temp_dest, dest.ch, prev_length);
    dest.ch = (char *)malloc(sizeof(char) * (prev_length + length + 1));
    memmove(dest.ch, temp_dest, prev_length);
    // dest.ch = (char *)realloc(dest.ch ,sizeof(char) * (prev_length + length + 1));
    long long j = prev_length;
    for (long long i = 0; i < length; i++)
    {
        // printf("copy %02X ", dest.ch[prev_length + i]);
        // printf("%c ", dest.ch[prev_length + i]);
        dest.ch[j + i] = src[i];
    }
    dest.length = prev_length + length;
}

StringType my_str_slice(StringType src, int start, int length)
{
    StringType dst;
    size_t min_length=length;
    if(start+length>=src.length){
        min_length=src.length-start;
    }
    dst.ch= (char *)malloc(sizeof(char) * min_length);
    dst.length=min_length;
    for(size_t i=0;i<min_length;i++){
        dst.ch[i]=src.ch[start+i];
    }
    return dst;
}

long long get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1e6) + (tv.tv_usec);
}