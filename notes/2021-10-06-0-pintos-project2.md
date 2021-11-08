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

<hr>

## 이해해야 할 것들

- stack 구조
- file & inode

<hr>

## Argument passing
- argv malloc시, 입력받은 argument들을 parsing하는 char형 token의 주소값을 저장한다.

## File system
- check_address 함수로 써도 문제 없을까?
- process_wait 함수에서 child list remove를 해도 괜찮을까?

<hr>

## Sab질 

1. `start_process()` 함수에서 `palloc_free_page()` 순서 이상하게 함
2. load에서 오류가 난줄알고 삽질삽질 filesys를 탓했지만 argument_passing 이상하게 한거엿
3. child_list에 넣을때 제대로 잘 넣어야했음 elem 잘쓰기
4. sema down시켜서 child process wait하는 타이밍 잘 생각하기 -> process_wait에 있어야하는걸 syscall_wait에서 했음
5. argumnet_passing 짤때: strlen 잘생각하고, iterater생각 잘하고, pointer의 농간에 놀아나지 말기(주소값인지 진짜 값인지), `uint32_t`를 암튼 쓰셈
6. sys call handler에서 thread_exit()을 call하는게 아니라 user program이 할 수 있도록 둬야한다. 스켈레톤을 신뢰하지 말자...
7. bad ptr를 어디 접근할 때 검사하고 exit시켜줘야한다. 그리고 lock release 안하고 가지고 죽지 않도록하기
8. `file_lock`은 read, write, open에만 필요하다.

(11.08 변경사항)
1. `process_exit()`에서 뭔가 가지고 있는 file_lock을 해제해줘야 할 것 같아서 해당 코드 추가함 -> multi-oom 에러코드 바뀜
2. `process_exit()`랑 `process_wait()` 에서 exit_sema 관련한 부분 처리해주니까 syn-write 통과함~!!!!!
3. 이제 남은건 multi-oom뿐.... 헤헤
4. `start_process()`에서 `palloc_free_page (file_name);` 추가하니까 총 8개 나오던 oom fail중에 하나 사라짐 -> 아무래도 메모리 효율이 딸리는듯?