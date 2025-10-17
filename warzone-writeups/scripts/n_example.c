#include <stdio.h>

int main()
{
    unsigned int len = 0;
    int n = 5;
    int *n_ptr = &n;
    printf("Hello World %d%n\n", n, &len);
    printf("Last printf had %d bytes\n", len);
    printf("can we replace n with something?\n%2$n");
    printf("n = %d\n", n);
}