src := $(shell echo src/*.c)

objs := $(src:src/%.c=obj/%.o)
objs_d := $(src:src/%.c=obj/%.do)

#set this to the desired executable name
exemain := myAlign

out := bin/$(exemain)

libs := -ldl -lm
includes := -Iinclude
debugflags := -g -DDEBUG
cflags := -O3

main : $(out)

$(out) : $(objs) 
	gcc -o $(out) $(objs) $(libs) $(includes) $(cflags)

obj/%.o : src/%.c
	gcc -c $< -o $@ $(libs) $(includes) $(cflags)

debug : $(out)_debug

$(out)_debug : $(objs_d)
	gcc -o $(out)_debug $(objs_d) $(libs) $(includes) $(cflags) $(debugflags)

obj/%.do : src/%.c
	gcc -c $< -o $@ $(libs) $(includes) $(cflags) $(debugflags)

clean : 
	rm obj/* bin/*
