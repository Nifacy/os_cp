c = compiled
co = compiled/ofiles
s = src

all: compiled/server compiled/client

# Object files

$(co)/connection.o: $(s)/connection/connection.c
	gcc -c $(s)/connection/connection.c -o $(co)/connection.o

$(co)/message.o: $(s)/message/message.c
	gcc -c $(s)/message/message.c -o $(co)/message.o

$(co)/message_io.o: $(s)/message/io/io.c
	gcc -c $(s)/message/io/io.c -o $(co)/message_io.o

$(co)/message_pair.o: $(s)/message/pair/pair.c
	gcc -c $(s)/message/pair/pair.c -o $(co)/message_pair.o

$(co)/message_storage.o: $(s)/message/storage/storage.c
	gcc -c $(s)/message/storage/storage.c -o $(co)/message_storage.o

$(co)/session.o: $(s)/session/session.c
	gcc -c $(s)/session/session.c -o $(co)/session.o

$(co)/session_storage.o: $(s)/message/storage/storage.c
	gcc -c $(s)/session/storage/storage.c -o $(co)/session_storage.o

$(co)/support.o: $(s)/support/support.c
	gcc -c $(s)/support/support.c -o $(co)/support.o

$(co)/server.o: $(s)/server.c
	gcc -c $(s)/server.c -o $(co)/server.o

$(co)/client.o: $(s)/client.c
	gcc -c $(s)/client.c -o $(co)/client.o

# Apps

compiled/server: $(co)/server.o $(co)/support.o $(co)/message_storage.o $(co)/session.o $(co)/session_storage.o $(co)/connection.o $(co)/message_pair.o $(co)/message.o $(co)/message_io.o
	gcc $(co)/server.o $(co)/support.o $(co)/message_storage.o $(co)/session.o $(co)/session_storage.o $(co)/connection.o $(co)/message_pair.o $(co)/message.o $(co)/message_io.o -lpthread -o compiled/server

compiled/client: $(co)/client.o $(co)/support.o $(co)/connection.o $(co)/message_pair.o $(co)/message.o $(co)/message_io.o
	gcc $(co)/client.o $(co)/support.o $(co)/connection.o $(co)/message_pair.o $(co)/message.o $(co)/message_io.o -lpthread -o compiled/client

# Utils

dirs:
	mkdir -p compiled
	mkdir -p compiled/ofiles

clear:
	rm /tmp/*.fifo
	rm compiled/*
	rm compiled/ofiles/*
	rm *.db

# gcc src/server.c src/support/support.c src/message/storage/storage.c src/session/session.c src/session/storage/storage.c src/connection/connection.c src/message/pair/pair.c src/message/message.c src/message/io/io.c -lpthread -o bin/server
# gcc src/client.c src/support/support.c src/connection/connection.c src/message/pair/pair.c src/message/message.c src/message/io/io.c -lpthread -o bin/client
# # gcc src/prog.c src/message/storage/storage.c -o bin/prog