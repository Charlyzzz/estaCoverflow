RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=%.o)

# Clean and compile .so
all: build/libconitos-estaCoverflow.so

build/libconitos-estaCoverflow.so: $(OBJS)
	$(CC) -shared -o "libconitos-estaCoverflow.so" $(OBJS)

%.o: ./%.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<" -lcommons

clean:
	$(RM) ./*.o
	$(RM) ./*.d
	$(RM) ./libconitos-estaCoverflow.so
	

install: all
	cp -u ./libconitos-estaCoverflow.so /usr/lib
	mkdir -p /usr/include/conitos-estaCoverflow/
	cp -u $(H_SRCS) /usr/include/conitos-estaCoverflow/

uninstall: clean
	rm -f /usr/lib/libconitos-estaCoverflow.so
	rm -rf /usr/include/conitos-estacoverflow

.PHONY: all clean install uninstall


