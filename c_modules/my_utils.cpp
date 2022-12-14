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

void my_strcpy(StringType &dest, char *src, int length)
{
    if (length <= 0)
        return;
    int prev_length = dest.length;
    char temp[prev_length];
    // resize & repopulate
    memcpy(&temp, dest.ch, prev_length);
    dest.ch = (char *)malloc(sizeof(char) * (prev_length + length + 1));
    memcpy(dest.ch, &temp, prev_length);
    int j = prev_length;
    for (int i = 0; i < length; i++)
    {
        // printf("copy %02X ", dest.ch[prev_length + i]);
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