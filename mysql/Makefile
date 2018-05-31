all:chat

chat:client.o main.o main2.o

client.o:client.c
	gcc -Wall -g client.c -o client.o -lpthread

main.o:main.c
	gcc -Wall -g -o main.o main.c  -I/home/nihou/Bureau/chat_reseau/mysql/include/ -L/home/nihou/Bureau/chat_reseau/mysql/lib -lmysqlclient
main2.o:main2.c
	gcc -Wall -o main2.o main2.c -I/home/nihou/Bureau/chat_reseau/mysql/include/ -L/home/nihou/Bureau/chat_reseau/mysql/lib -lmysqlclient

clean:
	rm -f *.o

