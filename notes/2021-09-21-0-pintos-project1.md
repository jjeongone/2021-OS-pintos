# [OS] Pintos Project1

## 스케쥴링

- FIFO: 선입선출 -> 먼저 요청한 작업부터 수행
- Shortest Job First -> 빠른 요청부터 수행

<br>

## Synchronization

- process 혹은 thread가 실행되는 시점을 조절하는 역할

<br>

## Deadlock

다음 네 가지 필요조건

- 상호배제
- 점유대기
- 비선점
- 순환대기

<br>

## Semaphore

- 두 개의 원자적 함수로 조작되는 정수 변수
  - `threads/synch.c` 에 작성되어 있음
- 멀티프로그래밍 환경에서 공유자원에 대한 접근성을 제한하는 방식
- 고전적인 해법으로 모든 교착상태(deadlock)를 해결하지는 못함
- 작동방식
  - S: 정수변수, P: 임계구역에 들어가기 전 실행, V: 임계구역에서 나올 때 실행
  - 한 process가 임계구역에 들어갈 때 P(S)를 실행해서 V(S)를 실행하기 전까지 다른 process의 접근을 막는 방식. 나중에 도착한 process는 P에서 기다리고 있음
  - 방법 1: 임계구역 진입때까지 반복문 수행 -> 비효율적, 어느 process를 임계구역에 넣을지 설정 불가능
    ```C
    P(S) {
    	while S <= 0; // 반복문
    	S--;
    }
    V(S) {
    	S++;
    }
    ```
  - 방법 2: 방법 1을 보완
    ```C
    P(S) {
      S--;
      if(S < 0) {
        // 해당 process를 재움 queue에 추가 -> 잠듦
      }
    }
    V(S) {
      S++;
      if(S <= 0) {
        // 재움 queue로부터 process를 제거 -> 깨어남
      }
    }
    ```

<br>

## busy waiting

자원을 얻기 위해 기다리는 것이 아니라 권한을 얻을 때까지 확인하는 것

- 단점: CPU를 낭비할 수 있음
- 대체방법: Sleeping
  - 실행중인 thread 정보를 wait queue에 넣고 CPU를 다른 thread에 양보
  - 권한 이벤트가 발생하면 wait queue에 있는 thread를 깨움

<br>

## Threads

### Struct Thread

- tid: thread마다 고유로 가지고 있는 id 값(int). 새로운 thread는 다음으로 큰 수로 할당됨.
- status: thread의 상태
  - **THREAD_RUNNING**: 실행중인 thread. 한 time에 하나만. `thread_current()`가 running thread를 반환해줌
  - **THREAD_READY**: 실행이 준비된 thread. double linked list인 **ready_list**에 들어있음
  - **THREAD_BLOCKED**: 무언가를 기다리고 있는 thread.(lock이 풀리기, intrerrupt가 실행되기) `thread_unblock()`을 통해 THREAD_READY 상태가 되기 전까지 스케쥴되지 않음.
  - **THREAD_DYING**: 다음 thread로 넘어가면 파괴될 thread
- name: thread의 이름(string)
- *stack: 모든 thread는 각각의 state를 기록하기 위해 stack을 가짐. CPU가 다른 thread로 넘어갈 때 thread의 stack pointer를 저장함
- priority: priority scheduling에서 사용될 priority
- allelem: thread를 전체 thread list에 link하는 데 사용. 모든 thread는 생성되거나 삭제될 때 list에 추가/삭제됨. `thread_foreach()` 함수를 통해 모든 thread를 iterate할 수 있음.
- elem: thread를 ready_list나 sema_down()과 같은 double linked list에 넣음.
- *pagedir: project2에서 사용됨
- magic: 임의로 설정된 값으로 threads/thread.c에 정의되어 있음. stack overflow를 detect하는 데 사용. `thread_current()`가 실행중인 thread의 magic member를 확인함. stack overflow는 이 값을 변화시킴. -> struct thread에 무언가를 추가했다면 magic을 끝으로 보내라(?)

### Thread Switching

threads/thread.c에 있는 `schedule()`에 의해 이루어짐. `thread_block()`, `thread_eixt()`, `thread_yield()`를 부르면 `schedule()`을 실행하는데, 실행하기 전에 thread를 running이 아닌 다른 상태로 바꿔줘야 한다.

<br>

## Synchronization

### Disabling Interrupts

가장 간단한 방법. interrupt가 꺼져있으면 그 어떤 thread도 running thread를 차지할 수 없음. 

### Semaphores
(*위에 설명 적혀있기는 함*) process가 실행될 때 semaphore를 down시켜서 다른 process의 접근을 막는 방식. 실행이 다 된 뒤에 semaphore를 up시키면 다음 process 실행 가능. initialize를 1로 하면 access를 resource로 controlling한다(?)

signaling mechanism을 이용하여 lock을 걸지 않은 thread도 signal을 이용하여 lock을 해제 가능.

### Locks(Mutex)

initial value가 1인 semaphore와 같음. **semaphore와 다른 점은 lock 되어있는 thread를 해제하기 위해서는 owner가 풀어주기를 기다려야 한다는 점이다.** 만약 이 조건이 방해가 된다면 semaphore를 사용하면 됨. 핀토스에서 lock이 recursive하지 않기 때문에 lock이 걸려있는 녀석한테 lock을 걸면 안됨.

lock을 건 thread만이 lock을 해제할 수 있음.

### Monitors(다시 읽어보기)

semaphore나 lock보다 더 higher level. synchronized된 data + monitor lock + condition variable 로 구성. 

공유자원을 사용하는 process가 실행중이면, ready queue에 새로 들어온 process를 넣고 끝나기까지를 기다리게 한다.

하나의 thread 내에서 다른 process를 실행시킬 때 사용된다?

- 공유자원 + 공유자원 접근 함수 + 2 Queue(배타동기, 조건동기)
- mutual exclusion queue
- conditional synchoronization queue

### Optimization Barriers(다시 읽어보기)

컴파일러가 memory across the barrier의 state를 추정하지 못하도록 함. 

<hr>

## Pintos Project 1 실행하기

전체 test case 실행하기

```bash
cd src/threads/build
make check
```

실행이 안되면 build 디렉토리 나갔다오면 되는듯..?(기묘함)

<br>

특수한 test case 하나 실행하기
```bash
cd src/threads
make clean
make
pintos -q run {test_name} // -q 옵션은 중요한지 모름
```

<br>

팁이라고 하면 전체 돌리는거랑 하나씩 돌리는거랑 git terminal 분리해두기

## Alarm Clock

> TODO: busy waiting을 피하도록 재구성하기!

### 기존 코드
- devices/timer.c
- `timer_sleep()` 
  - 최소한 x만큼의 시간이 흐른 뒤에 thread를 실행하는 코드
  - thread 0은 10 ticks동안 sleep하고, thread 1은 20 ticks동안 sleep하는 그런 구조

### TODO
- devices/timer.c 코드 정독

### idea

```C
void timer_sleep (int64_t ticks) {
  int64_t start = timer_ticks();
  ASSERT(intr_get_level() == INTER_ON);
  while(timer_elapsed(start) < x) {   // x값을 thread로부터 계산?

  }
  thread_yield();
}
```
thread가 생성될 때 created_ticks를 저장해 둔다. timer_sleep 함수에서 

<br>

## Priority Scheduling

> TODO: lock에 대한 priority donation 구현하기. 모든 케이스에 대한 priority scheduling을 구현해야 함.

### Skeleton code
- threads/thread.c
- `thread_set_priority(int *new_priority*)`
  - thread의 priority를 *new_priority*로 업데이트. current thread가 더이상 highest priority를 가지지 않으면 yield.
- `thread_get_priority(void)`
  - current thread의 priority를 return. priority donation의 경우에는 donated된 priority를 반환함

### Description

현재 실행중인 thread보다 higher priority의 thread가 ready list에 들어오면 새로운 thread에 process를 양보한다(? < yield the processor to the new thread 라고 되어있음)

유사하게 lock이나 semaphore, condition variable에 의해 waiting하고 있을 때, 우선순위가 높은 친구가 먼저 깨어남

thread는 priority를 언제나 올리거나 내릴 수 있음. 근데 priority를 낮추는 것은 바로 CPU를 양보하겠다는?

priority 초기값은 특별한 이유 없이는 31로 설정(min: 0 / max: 63)

**priority inversion**: H > M > L 인 thread가 있다고 가정했을 때 H가 L을(lock같은 작용에 의해) 기다려야 하고 M이 ready list에 존재할 때 H는 L을 기다려야 하는데 L은 priority가 낮으니까 CPU를 할당받지 못함. 그래서 H가 L에게 lock이 해제될 떄까지 priority를 donate해주는 방식으로 해결할 수 있음

**priority donation**: 구현할 때 priority donation이 일어날 수 있는 경우들을 생각해서 잘 계산해야 한다. multiple donation이 존재하는데, 이는 여러 priority가 하나의 thread에 donate되는 경우. nested donation도 고려해야 한다. 만약 L이 M을 기다리고 M이 L을 기다려야 하면 M과 L 모두 H의 priority를 통해 실행되어야 함.

**Lock**

여기 함수를 보면 lock이 존재하고, 이 lock의 semaphore를 `sema_down()` 함수를 통해 내려서 lock->holder를 lock을 가지고 있는 thread를 넣어줌

```C
void
lock_acquire (struct lock *lock)
{
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (!lock_held_by_current_thread (lock));

  sema_down (&lock->semaphore);
  lock->holder = thread_current ();
}
```
그니까 lock을 누군가(thread)가 소유하고 있고, 그 lock을 이용하려면 lock을 release해줘야 하는 것. 근데 lock은 owner만 해제할 수 있기 때문에 priority donation에서 `lock_acquire` 요청이 들어왔을 때 priority를 검사해서 더 낮은 priority인 thread가 lock을 차지하고 있으면 priority donation을 일으켜서 (낮은 priority의 thread가)`lock_release`가 되도록 하고 release되면 사용한 priority를 반납, 높은 priority의 thread가 실행됨.

### idea

1. priority scheduling 구현
   1. ready list를 priority 순으로 정렬
   2. priority 순으로 pop해서 실행(기존 로직)
2. priority donation
   1. 일반적인 priority donation: `lock_acquire`을 수정해서 `sema_down`을 실패하면, lock을 이미 사용중이라는 뜻이니까 여기서 `thread_get_priority`로 priority를 check. 그래서 `thread_donate_priority`를 이용해서 donation을 일으킴. -> 여기서 궁금한 점은 donatoin을 일으켜주면 알아서 실행이 되나? 아닌 것 같은데 ready list의 순서를 업데이트하고, 현재 thread를 yield를 시키는 과정도 필요할 것 같은디..?
   2. multiple priority donation: 
   3. nested priority donation: donation받은 priority를 stack으로 저장해두면? donation 관계를 pointer로 연결해두는게 더 맞을 수도 있을 것 같음. 왜냐하면, L < M < H 순의 priority를 가지고 있고, M이 L에 lock, H가 M에 lock이 있을 때 M이 L한테 donation해서 L(M)이 되는데, H가 M한테 donation을 하면 M(H)가 됨. 그러면 L을 다시 L(H)로 업데이트해줘야 하는데 이거를 어떻게 처리할 것인가. 

**구현 목록**

- priority_check(): priority를 비교해서 boolean을 return
- thread_donate_priority(): priority donation을 일으킴

<br>

## Advanced Scheduler

> TODO: 4.4BSD scheduler과 유사한 multilevel feedback queue scheduler(MLFQS)를 만들어보자!

### Description

priority scheduler과 유사하게 priority를 기반으로 실행할 thread를 선택함. 근데 이제 priority donation은 일어나지 않음. -> priority scheduler에서 priority donation을 제외한 상태로 advanced scheduler 짜는거 추천함.

priority에 따른 ready queue를 이용해서 MLFQS가 priority를 자동으로 관리할 수 있도록 함

thread의 priority를 4 ticks마다 refresh

pintos가 실행될 때 scheduling algorithm을 선택할 수 있도록 코드를 작성해야 함.

- 기본적으로는 priority scheduler를 사용하되, `-mlfqs` kernel option을 통해 4.4BSD scheduler를 선택할 수 있음
- threads/thread.h의 `thread_mlfqs`를 통해 작동함

4.4BSD scheduler가 활성화되면 thread는 priority를 직접적으로 control하지 않는다. `thread_create()`는 무시되어야 하고, `thread_set_priority()`나 `thread_get_priority()`는 scheduler에 의해 설정된 현재의 triority를 반환한다.

### B. 4.4BSD Scheduler


여러개의 ready-to-run thread에 대한 queue를 가지고 있음. 각 queue는 다른 priority의  thread를 가지고 있음. 실행할 때 scheduler는 hightest-priority의 non-empty queue에서 thread를 고름. hightest-priority queue에 여러개 들어있으면 round robin 순으로. -> round robin도 우리가 구현해야 하는 부분인가..?

**Niceness**: [-20, 20] 범위의 값을 가짐. 양수는 priority를 감소시킴. negative는 다른 thread로부터 CPU time을 가져오려고 함

**Calculating Priority**: 총 64개의 priority -> 64개의 queue. thread priority는 thread initialization때 계산되고, every fourth clock tick마다 다시 계산됨. recent_cpu가 thread가 최근에 쓰인 time의 추정값을 가지고 있기 때문에 starvation을 막을 수 있음(최근에 CPU가 안쓰인 녀석이면 recent_cpu값이 0이 되서 priority가 높게 계산될 수밖에 없음)

$$priority = PRIMAX - (recentcpu / 4) - (nice * 2)$$

**Calculating recent_cpu**: *최근에* thread가 사용한 CPU time 값. exponentially weighted moving average를 사용해서 계산.

`thread_get_recent_cpu` 함수에서 아래 수식을 구현해야 함

- recent_cpu 초기값은 0
- 매 timer tick마다 running thread의 recent_cpu는 1씩 증가함
- 매 초마다 모든 thread의 cpu는 아래 식을 이용해서 업데이트됨
$$recentcpu = (2*loadavg)/(2*loadavg + 1) * recentcpu + nice$$

**Calculating load_avg**: 지난 분(minute) 동안 실행된 avg number of thread를 계산함. 특정 thread에 대한 것이 아니라 system level의 계산. 

`thread_get_load_avg` 함수에서 아래 수식을 구현해야 함

- ready_theads: running중이거나 ready to run인 thread의 개수(idle thread 미포함). 초기값은 0
- 매 초마다 아래 식으로 업데이트됨.
$$loadavg = (59/60)*loadavg + (1/60)*readythreads$$

**fixed-point**: pintos는 floating point(부동소수점)을 지원하지 않기 때문에(복잡하고 kenel을 느리게 만들 수 있다는 이유에서), 계산에 있어서 유의해야 한다. 

### ide

<hr>

## Question

1. threads/thread.c `thread_create()` (line: 194): 저 void 부분 코드 이해안됨
    ```C
    ef->eip = (void (*) (void)) kernel_thread;
    ```
2. semaphore를 1로 initialize했을 때 뭐가 다른거지?(semaphore를 0으로 초기화 했을 때 / lock 과 비교하기)

<hr>

## 결과

### 2. Priority Scheduling

donation 순서

> 임의의 thread A(a) < B(b) < C
> lock a, b

1. B를 A의 donated_list에 추가
2. lock a의 waiters에 B 추가
3. B가 lock a의 값을 기다리고 있음을 저장
4. B->A donated_priority를 비교해서 priority donation
5. A가 다른 lock을 기다리는지 확인 -> 여기서는 null -> yield

6. C를 B의 donated_list에 추가
7. lock b의 waiters에 C 추가
8. C가 lock b의 값을 기다림
9. C->B donated_priority를 비교해서 donation
10. B가 마침! lock a를 기다림 -> a의 holder의 donated_priority와 B의 것을 비교 -> donation -> lock의 holder가 기다리는 lock 주소가 NULL일때까지 반복

<hr>

## 후기

### 어이없는 핀토스

1. `printf`가 ticks를 잡아먹어서 alarm-multiple이 안돌아갔음..
2. alarm-simultaneous가 기묘하게 ticks가 미묘하게 더 걸려서 0tick만에 실행되어야 하는게 1tick을 잡아먹어서 로직 수정하니까 제대로 돌아감.. *코드를 효율적이게 짜자!!*
3. 함수 호출을 최소화하기! 메크로로 돌리면 더 효율적이다!

### idle

1. idle은 아무것도 모른다. 계산에서 잘빼기
