#��ִ���ļ�
target = get_city_from_sohu
#Դ�ļ�
SRCS = cJSON.c city_sohu.c get_city_from_sohu.c
COMMAND=-L/yuan/pro/https/lib -lcurl -lssl -lcrypto -lz -lm -lbngo_aes -lbngo_info -lini -lszhy_get
#����Ŀ��
objects = $(SRCS:.c=.o)
#ָ�������
CC = mips-linux-gnu-gcc
$(target):$(objects)
	mips-linux-gnu-g++ -o $(target) $(objects) $(COMMAND)
$(objects): cJSON.h city_sohu.h
	$(CC) -c $(SRCS)
.PHONY:clean
clean:
	rm -rf $(target) $(objects)