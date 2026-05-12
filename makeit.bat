zcc +radio86 -lndos -unsigned -O3 ssb.c -o ssb -pragma-define:CLIB_EXIT_STACK_SIZE=0 -pragma-define:CLIB_OPEN_MAX=0 --list -m -create-app
