# [OS] Pintos Project2

## Overall

base code -> user program의 loading이랑 running은 이미 구현되어 있음. 이제 I/O나 interactivity는 불가능한데, 이번 project에서 구현하게 될 것.

goal: user program이 실행될 수 있도록 하는 것

build services for user program to use
- command-line argument passing
- process termination messages
- system calls
- write protection on executable files in use

Requirements
- process termination messages
- argument passing (1)
- system call
- denying writes to executable

modify file
- src/userprog

To understand
- virtual memory: structure, accessing method
- thread, process: relation with parent, child
- system call handler
- basic file system: file과 inode의 관계성

<hr>

## Process termination messages

user process가 terminate하면 process의 이름과 exit code를 출력한다.(**kernel processs나 halt system call은 출력하지 않음!!**) 

```c
printf("%s: exit(%d)\n", process_name, exit_code);
```

## Argument passing

```bash
ls -l foo bar
```

위의 커멘드를 실행시켰을 때 `ls`라는 program file을 `-l`, `foo`, `bar` argument를 이용하여 실행한다.

caller의 stack pointer는 esp를 통해 접근 가능 -> struct intr_frame 안에 존재함.

`process_execute()` 를 확장해서 새 process의 argument passing에 사용할 수 있다.

- word를 space로 나눠서 제공: [0]program name, [1]first argument , ...
- multiple space는 single space 취급
- command line argument에 reasonable한 제한을 둘 수 있음

## System call

user process가 kernel에 접근하려고 할 때 system call을 일으킨다.

`syscal.h`, `syscal.c` 파일에 코드를 구현하면 됨.

system call number에 대한 정보는 `lib/syscal-nr.h`에 나와있음.

- user process manipulation: user process가 다른 process를 control할 수 있게 해줌.
    - halt(), exit(), exec(), wait()
- file manipulation: 기본적인 file system은 `filesys/filesys.c, file.c`에 구현되어 있음. system call을 제공해서 user program이 function에 접근할 수 있도록 해야 한다.

## Denying writes to executable

running중인 program file에 write하려는 시도를 무시하도록 해야 함

`file_deny_write()`과 `file_allow_write()`를 적절한 타이밍에 부르기

<hr>

## virtual memory

## thread, process

## system call handler

## basic file system