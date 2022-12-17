#include "../include/data_structures.h"
#include "../include/headers.h"
int main(int argc, char const *argv[])
{
    Queue *q = createQueue();
    char *s1 = "AAA";
    char *s2 = "BBB";
    char *s3 = "CCC";
    char *s4 = "DDD";
    char *s5 = "EEE";
    enqueue(q, s1);
    enqueue(q, s2);
    enqueue(q, s3);
    enqueue(q, s4);
    enqueue(q, s5);
    char *temp = 0;
    while (dequeue(q, &temp))
    {
        printf("%s", temp);
    }

    return 0;
}
