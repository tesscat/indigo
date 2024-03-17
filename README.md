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
