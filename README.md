sane coding conventions:
- CONSTS\_IN\_SCREAMING\_SNAKE
- variables\_in\_snake
- TypesInPascal
- functionsInCamel
- membersInCamel

#### Plan:
there is no proper plan.  
I need to:
- implement a _page_ allocator/tracker atop the phys address allocator, with which I can:
- - ask for N contiguous bytes of memory, optionally at a specific physical address
- - specify which virtual address this should be mapped to
- - handle freeing of pages

### TODOs/FIXes:
- Build a proper page table for the kernel so that it doesn't identity-page _everything_. This will require:
- - a bit of constantly mapped "scratch space" so that we can point the kernel page dir entries to + write page dirs upon so that we can access Whole Memory so that we can build process page tables
- - this needs a slab allocator for like a `PageEntryBase scratch[0xbignumber]` tho so i'm procrastinating
