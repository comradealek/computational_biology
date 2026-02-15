src := $(shell echo src/*.c)
src2 := $(shell echo src2/*.c)

objs := $(src:src/%.c=obj/%.o)
objs2 := $(src2:src2/%.c=obj2/%.o)

out := bin/main
out2 := bin2/main

libs := -ldl -lm
includes := -Iinclude
cflags := -O3 -g

main : $(out)

$(out) : $(objs) 
	gcc -o $(out) $(objs) $(libs) $(includes) $(cflags)

obj/%.o : src/%.c
	gcc -c $< -o $@ $(libs) $(includes) $(cflags)

main2 : $(out2)

$(out2) : $(objs2)
	gcc -o $(out2) $(objs2) $(libs) $(includes) $(cflags)

obj2/%.o : src2/%.c
	gcc -c $< -o $@ $(libs) $(includes) $(cflags)

clean : 
	rm $(objs) bin/main
