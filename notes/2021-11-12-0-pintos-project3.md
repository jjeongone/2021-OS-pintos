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

### Design Report
- analysis on 7 requirements
- design for 7 requirements
- Analysis: each requirement's meaning, implementation of original pintos, problems in the original pintos, necessity of each requirement

> 7 requirement가 뭐시기여?

> `0.Current Implementation Analysis` 에서 각각의 requirement에 대해 쓰고, 의미, 현재 구현, 문제점, 중요성 쓰고, `1.` 부터해서 `7.` 까지 data structure, algorithm을 포함한 design 작성. algorithm에서 각 requirement를 어떻게 만족하는지도 설명해야 함. 

> How to complete each implementation objective in the document 요거는 뭐쓰라는거지?

### Requirements
_Lab3 ppt에 따르면, (근데 이거 아닌 것 같은데.. 9페이지이지 않을까 하는 추측)_

1. frame table
2. lazy loading
3. supplemental page table
4. stack growth
5. file memory mapping
6. swap table
7. on process termination

<hr>

## Memory Terminology

### Pages == virtual page
      31                12 11        0
      +-------------------+-----------+
      |    Page Number    |   Offset  |
      +-------------------+-----------+
              Virtual Address

continuous region of virtual memory: 4,096bytes(page size) & page-aligned

각 process는 virtual address의 PHYS_BASE아래에 independent한 user page를 가진다. virtual address의 PHYS_BASE는 physical address의 0에 접근한다. (즉, virtual address PHYS_BASE + 0X1234가 physical address의 0X1234에 접근한다는 뜻) -> `physical address + PHYS_BASE == kernel virtual address`

kernel virtual page는 global이다. 

> kernel virtual memory가 그냥 kernel에 대한 virtual memory라는건가? 지금껏 우리가 써온 kernel memory가 애초에 kernel virtual memory이고, 실제로 memory상의 주소로 접근할 때에는 kernel virtual address값에서 PHYS_BASE를 뺀 값인거고?

<br>

### Frames == physical frame, page frame
      31                12 11        0
      +-------------------+-----------+
      |    Frame Number   |   Offset  |
      +-------------------+-----------+
              Physical Address

continuous region of physical memory: page-size & page-aligned

80x86은 physical address에 직접적으로 접근하는걸 막음. 그래서 kernel virtual memory를 physical memory에 mapping해줘야 함. 

<br>

### Page Tables

      virtual address - [page table] -> physical address

를 수행하는 data structure. virtual address의 `Page Number`을 기준으로 `Frame Number`를 대응시켜준다. `Offset`은 그냥 가져가는듯? 

<br>

### Swap Slots

continuous, page-size region of disk space in the swap partition. 

<hr>

## Resource Management Overview

### 1. Supplemental page table 

page table -> page fault handling 가능하게

### 2. Frame table 

효율적인 eviction policy 짜기

### 3. Swap table 

swap slot의 usage를 track?

### 4. Table of file mappings 

위의 4개 data structure 반드시 분리된 4개로 안짜도 됨. 적당히 related되도록 짜면 편함.

data structure에 뭐넣을지, 범위(local, global)

non-pageable memory에 저장하면 구현이 간단해짐. 

가능한 data structure: arrays, lists, bitmaps, and hash tables -> array 간단하지만 memory 낭비. list는 전체를 검사해야 하는 단점. 근데 insertion/deletion은 list가 더 유리함

**bitmap data structure**: resource의 usage를 track하는데 편리함. fixed size로 구현되어있지만, 필요하면 수정해서 쓰기

**hash table data structure**: insertion과 deletion이 효율적임

<hr>

## Managing the Supplemental Page Table

1. page fault가 발생했을 때 kernel이 supplemental page에 있는 fault를 일으킨 virtual page를 살펴서 어떤 데이터가 있어야하는지를 확인..? (Most importantly, on a page fault, the kernel looks up the virtual page that faulted in the supplemental page table to find out what data should be there)
2. process terminate이 일어날 때 kernel이 supplemental page table을 설계함 -> 어느 resource를 free시킬지 정함

page fault handler에서 supplemental page table을 가장 많이 씀. project2까지는 page fault가 bug로 취급받았다면, 이제는 그저 page가 file로부터 받아오거나 swap되어야 하는 의미 -> 구현해야함

<br>

### Implementation

supplemental page table을 organize하는 방법
- terms of segment
- terms of page

page_fault 구현하기 ("`userprog/exceptino.c`")
1. supplemental page table에서 fault된 page를 찾아낸다. memory reference가 valid한 경우에, page에 들어갈 data에 대해 supplemental page table을 entry로 사용함.(file system에 있거나 swap slot에 있거나 all-zero page) supplemental page table이 1)user process가 접근하고자 하는 주소에서 어느 데이터도 기대하지 못함 2)page가 kernel virtual memory에 있음 3)read-only page에 write를 시도 일 경우 invalid. invalid한 접근일 경우 process를 terminate하고 resource free시키는 과정 필요
2. obtain a frame to store the page. sharing을 구현하면, 필요한 데이터가 이미 frame에 있을 것.
3. fetch the data into the frame by reading it from the file system or swap, zeroing it, etc. sharing을 구현하면, 필요한 page가 이미 frma에 있을 것.
4. point the page table entry for the faulting virtual address to the physical page 

<hr>

## Managing the Frame Table

user page을 contain하는 각 frame의 one entry를 contain한다(??). page에 대한 pointer를 포함.

> 그니까 각 frame table이 각 user page를 포함하는 frame에 대해 하나씩 entry를 가져서, 이녀석이 또 page에 대한 pointer를 가진다는 말인가?

frame table을 이용하면 eviction policy를 효율적이게 구성할 수 있다 -> no frames are free일때  evict할 page를 선택하도록 해서

user pages에서 사용되는 frames는 "user pool"에 존재해야 하기 때문에 `palloc_get_page(PAL_USER)`를 호출해서 사용해다 한다. 이때 반드시 PAL_USER를 사용해야 한다(안쓰면 "kernel pool"에 allocate할 수 있음 -> test fail할 수 있음)

frame table의 중요한 기능 중 하나는, unused frame을 가지는 것. frame이 꽉차있을 때(none is free) frame속의 page를 evict해야 함

swap을 allocat하지 않고는 frame을 evict할 수 없는데, 여기서 또 swap이 꽉찬 경우에는 kernel panic. 실제 OS에서는 이 상황을 극복하는 mechanism, policy 이런게 존재함.

<br>

### eviction process
1. page replacement algorithm을 이용하여 evict시킬 frame을 선택한다. page table의 **accessed**와 **dirty**같은 것들을 이용
2. frame에 대한 reference를 제거한다.
3. 필요하면, file system에 page를 쓰거나 swap

<hr>
   
## Accessed and Dirty Bits

80x86 하드웨어는 page replacement algorith에 도움을 줄 수 있는 bit쌍(PTE에 존재함)들을 제공한다.

- any read or write to a page: `accessed = 1` (CPU가 함)
- any write: `dirty = 1` (CPU가 함)

값을 0으로 초기화하는 작업은 CPU가 안하고 OS가 한다.

두개 이상의 page가 같은 frame을 참조하고 있을 때 alias를 고려해야 한다. aliased frame이 accessed되었을 때 accessed and dirty bit는 하나의 PTE에서만 update된다.(다른 참조하고 있는 녀석들의 PTE는 update되지 않음)

pintos에서는 모든 user virtual page가 kernel virtual page에 aliased 되어 있음. 요거를 잘 manage해줘야 함. 예를 들어, 두 address의 accessed and dirty bit을 모두 check하고 update하는 방식이 있다. 혹은 user data를 user virtual address만을 통해 접근할 수 있도록 하면 kernel은 이 문제를 피할 수 있다.

> 전자로? (간단해보임)

<hr>

## Managing the Swap Table

swap table은 in-use와 free swap slot을 track한다.

- picking unused swap slot: evict a page from its frame to the swap partition
- freeing a swap slot: page가 read back이거나, process의 page swap이 terminated되거나

BLOCK_SWAP을 사용 -> block device for swapping

"`vm/build`"에서 `pintos-mkdisk swap.dsk --swap-size=n` 커멘드 사용하면, swap.dsk를 만드는데, 이녀석이 extra disk로 사용된다. 

swap slot은 allocate lazily해야함 -> eviction에 의해 요구된다

read back into a frame일때 swap slot을 free 시켜준다

> 사실 잘 모르겠음..,,

<hr>

## Managing Memory Mapped Files

mmap을 이용하여 console에 file을 출력하는 예시
 	
``` C
#include <stdio.h>
#include <syscall.h>
int main (int argc UNUSED, char *argv[]) 
{
  void *data = (void *) 0x10000000;     /* Address at which to map. */

  int fd = open (argv[1]);              /* Open file. */
  mapid_t map = mmap (fd, data);        /* Map file. */
  write (1, data, filesize (fd));       /* Write file to console. */
  munmap (map);                         /* Unmap file (optional). */
  return 0;
}
```
memory mapped file에 의해 어떤 memory 가 사용되고 있는지를 track할 수 있어야 한다. mappped region에서 page fault를 handle할 때 사용됨(mapped file이 overlap을 하지 않도록)


<hr>

## Requirements

### 1. Paging

*modify file*: userprog/process.c `load_segment()`

LRU와 같은 global page replacement algorithm 만들기

parallelism을 고려해야 한다.

- `page_read_bytes == PGSIZE`: page를 요구해야 한다
- `page_zero_bytes == PGSIZE`: disk로부터 읽어올 필요가 없음. first page fault를 일으켰을 때 0값을 가지는 새 페이지를 만든다?
- 그 외: underlying file로부터 initial part of page를 읽음. 나머지는 zero(initial part of the page is to be read from the underlying file and the remainder zeroed.)

<br>

### 2. Stack Growth

stack access가 일어날 때 additional page를 allocate한다.

기존 -> user virtual address space에서 single page를 차지함 -> 한정된 자원 -> 근데이제 늘려야하는

<br>

### 3. Memory Mapped Files 

memory mapped files를 구현해보자!

- mapid_t _mmap (int fd, void *addr)_: process의 virtual address에 fd로 열린 file을 mapping한다.
  - `file_length != n * PGSIZE`: 남은 byte들에 대해서 값을 0으로 설정해주고, disk에 write할 때 무시할 수 있도록 한다.
  - `success`: return mapping ID
  - `fail`: return -1 
    - 0 byte length인 file일 때
    - addr이 not page-aligned
    - overlaps any existing set of mapped pages
    - executable load time일 때
    - addr == 0: pintos는 0인 page를 unmapped된 page로 취급
    - fd == 0 || fd == 1: console input, output은 mapping 안함

- void _munmap (mapid_t mapping)_: mapping에 의해 설계된 mapping을 unmap한다.

mapping된 것들은 process_exit할 때 unmap되어야 한다. mapping이 unmapped되면, process에 의해 written된 page를 file에 written back해줘야 한다.

> 이말이 그니까 process에서는 write을 virtual address에 할당된 page에다가 하는거고, 이게 unmapped될 때 비로소 file에 쓴다는 말일까? -> 요거 확인해보기

file의 close이나 remove는 mapping에 영향을 주지 않는다. 한번 mapping되면 munmap이 불리거나 process exit이 되기 전까지 유효하다.

두개 이상의 process가 같은 file을 map했을 때: 그냥 같은 physical page를 보도록 하면 됨. 

> 그러면 mmap이 copy-on-write 기능을 제공해야한다는 말임? 이거까지 하면 넘 헤비한디;; -> 확인해보기

<br>

### 4. Accessing User Memory 

system call을 호출했을 때 user memory에 접근할 수 있도록 코드를 수정해야 한다. user memory에 접근할 때 kernel은 page fault을 handle할 수 있거나, prvent해야 한다. device driver에 의해 acquired된 lock을 포함하는 resource를 hold하고있는 동안 page fault를 막아야 한다.

이거를 하기 위해서는 access가 발생하는 코드와 page eviction code의 cooperation이 요구된다. frame table을 pagerk evict되면 안되는 것을 포함하는지를 record 하도록 확장할 수 있다.(pinning, locking) Pinning은 page replacement algorithm의 선택을 제한할 수 있음. 필요하지 않은 곳에서 pinning page를 피해라는데 무슨말이지.

> pinning을 사용한다면, bitmap을 이용해서 eviction algorithm이 돌아갈 때 evict되면 안되는 녀석들을 확인해서 잘 걸러주는 방식으로 구현하면 될듯?

<hr>

## 잘 모르겠는 어쩌구들

- lazily load pages가 무슨말이지?
- clock algorithm 제대로 이해하기!

<hr>

## 즐겁고 신나는 VM 구현

### 고민
- `#ifdef VM` 위치 고민해보기
- `src/Makefile.build`에서 vm_SRC를 추가했다. 안 돌아가면 의심해보기
- `#include <hash.h>` lib에 있는 헤더파일은 이렇게 써도 되는건가?
- page에서 저장한 file에 대한 정보를 이용해서 disk로부터 file을 읽어서 frame의 kernel virtual address에 저장

- `userprog/process.c`의 `load_segment` 함수: frame table을 `palloc_get_page(0)`으로 할당해도 됨?

### TODOs
- pintos 시작할 때 frame `hash_init` 불러줘야한다
  - frame_init() 함수 만들기
  - supp_page_init() 함수도 만들기
- struct 할당할때 malloc 사용하는걸로 바꾸기

### 알아야 할 것들
- `load_segment()` 함수에서 page 할당 frame 할당 어떻게 구현하면 좋은지
- 결과적으로 frame이랑 page가 서로의 주소를 알아야 하는데 이거 언제 연결시켜줘야 하는지
- lazy loading?

### 이해한 것
- user program은 pages(supplemental page table)에서 해당하는 file의 정보를 가지고 있음
- 아직 frame 할당을 안받음 == PM에 올라오지 않았음
- user program이 실행되다가 이 file에 접근하려면 page->frame 주소가 비어있음 -> page faule가 발생함
- 그제서야 lazy load로 인해서 frame 할당이 이루어짐
- 이때 frame에 자리가 없으면 swap이 일어나는 것
- load가 완료되면 그제서야 다시 user program을 실행시켜줌

> 그러면 애초에 thread가 init될 때 load시키는 file에 대해서는 frame과 page 연결이 어느 시점에 일어나야 하는가?

## Design

### supplemental page table를 어떻게 구현?

thread -> page list를 구현

page
- mapping된 fd?
- 

### frame table을 어떻게 구성할 것인가(Data Struct)

- frame table에서는 hash로 관리가 안 필요한 것 같음!

### swap table을 어떻게 구현?

