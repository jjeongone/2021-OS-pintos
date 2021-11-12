# [OS] Pintos Project3

## Overall

### Key Terms
- Page / Frame
- Page table / Frame table
- Page fault
- Page fault handler
- Page replacement algorithm (e.g. LRU, Approximate LRU.) • Eviction policy (Accessed / Dirty bit)
- Swap
- Stack growth
- Memory-mapped I/O (mmap, munmap system call)



<hr>

## Data Structure

### 1. Supplemental page table 

page table -> page fault handling 가능하게

### 2. Frame table 

효율적인 eviction policy 짜기

### 3. Swap table 

swap slot의 usage를 track?

### 4. Table of file mappings 

<br>

## Implementation

위의 4개 data structure 반드시 분리된 4개로 안짜도 됨. 적당히 related되도록 짜면 편함.

data structure에 뭐넣을지, 범위(local, global)

non-pageable memory에 저장하면 구현이 간단해짐. 

가능한 data structure: arrays, lists, bitmaps, and hash tables -> array 간단하지만 memory 낭비. list는 전체를 검사해야 하는 단점. 근데 insertion/deletion은 list가 더 유리함

bitmap data structure를 지원함. resource의 usage를 track하는데 편리함. fixed size로 구현되어있지만, 필요하면 수정해서 쓰기

<hr>

## Supplemental Page Table

1. page fault가 발생했을 때 kernel이 supplemental page에 있는 fault를 일으킨 virtual page를 살펴서 어떤 데이터가 있어야하는지를 확인..? (Most importantly, on a page fault, the kernel looks up the virtual page that faulted in the supplemental page table to find out what data should be there)
2. process terminate이 일어날 때 kernel이 supplemental page table을 설계함 -> 어느 resource를 free시킬지 정함

page fault handler에서 supplemental page table을 가장 많이 씀. project2까지는 page fault가 bug로 취급받았다면, 이제는 그저 page가 file로부터 받아오거나 swap되어야 하는 의미 -> 구현해야함

<br>

## Implementation

supplemental page table을 organize하는 방법
- terms of segment
- terms of page

page_fault 구현하기 ("userprog/exceptino.c")
1. supplemental page table에 fault된 page를 둔다. memory reference가 valid한 경우에, page에 들어갈 data에 대해 supplemental page table을 entry로 사용함. supplemental page table이 1)user process가 접근하고자 하는 주소에서 어느 데이터도 기대하지 못함 2)page가 kernel virtual memory에 있음 3)read-only page에 write를 시도 일 경우 invalid. invalid한 접근일 경우 process를 terminate하고 resource free시키는 과정 필요
2. obtain a frame to store the page
3. fetch the data into the frame by reading it from the file system or swap, zeroing it, etc.
4. point the page table entry for the faulting virtual address to the physical page 

<br>

## Managing the Frame Table

user page을 contain하는 각 frame의 one entry를 contain한다(??). page에 대한 pointer를 포함. frame table을 이용하면 eviction policy를 효율적이게 구성할 수 있다. -> no frames are free일때  evict할 page를 선택하는 상황에서

kernel pool에 allocating하지 않도록 PAL_USER를 사용해야 한다. test fail의 요인이 될 수 있으니 주의.

frame table의 중요한 기능 중 하나는, unused frame을 가지는 것. frame이 꽉차있을 뗴(none is free) frame속의 page를 evict해야 함

swap을 allocat하지 않고는 frame을 evict할 수 없는데, 여기서 또 swap이 꽉찬 경우에는 kernel panic. 실제 OS에서는 이 상황을 극복하는 mechanism, policy 이런게 존재함.

eviction의 progress
1. page replacement algorithm을 이용하여 evict시킬 frame을 선택한다. page table의 `accessed`와 `dirty`같은 것들을 이용
2. frame에 대한 reference를 제거한다.
3. 필요하면, file system에 page를 쓰거나 swap
   
## Accessed and Dirty Bits

replacement algorithm에 도움? 

alias에 신경을 써야한다 -> 

pintos에서 user virtual page가 kernel virtual page에 존재. code가 accessed랑 dirty bits를 모두 확인하고 check&update할 수 있어야 함.

## Managing the Swap Table

swap table은 in-use와 free swap slot을 track한다.

## Managing Memory Mapped Files

<hr>

## Requirements

### 1. Paging

*modify file*: userprog/process.c `load_segment()`

LRU와 같은 global page replacement algorithm 만들기

parallelism을 고려해야 한다.

- `page_read_bytes == PGSIZE`: page를 요구해야 한다
- `page_zero_bytes == PGSIZE`: disk로부터 읽어올 필요가 없음. first page fault를 일으켰을 때 0값을 가지는 새 페이지 를 만든다?
- 그 외: underlying file로부터 initial part of page를 읽음. 나머지는 zero(initial part of the page is to be read from the underlying file and the remainder zeroed.)

### 2. Stack Growth

stack access가 일어날 때 additional page를 allocate한다

### 3. Memory Mapped Files 

### 4. Accessing User Memory 

