#include <stdio.h>

int main()
{
    unsigned int len = 0;
    int n = 5;
    printf("Hello World %d%n\n", n, &len);
    printf("Last printf had %d bytes\n", len);   
}