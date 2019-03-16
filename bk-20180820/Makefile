#可执行文件
target = get_city_from_sohu
#源文件
SRCS = cJSON.c city_sohu.c get_city_from_sohu.c cacertinmem.c
COMMAND=-L/yuan/pro/https/lib -lcurl -lssl -lcrypto -lz -lm -lbngo_aes -lbngo_info -lini -lszhy_get
#依赖目标
objects = $(SRCS:.c=.o)
#指令编译器
CC = mips-linux-gnu-gcc
$(target):$(objects)
	mips-linux-gnu-g++ -o $(target) $(objects) $(COMMAND)
$(objects): cJSON.h city_sohu.h
	$(CC) -c $(SRCS)
.PHONY:clean
clean:
	rm -rf $(target) $(objects)