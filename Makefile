all: server client 

server: sample_server.c threads_manager.c pseudos.c card.c players.c
	gcc -w $^ -pthread -o server

client: sample_client.c card.c client_game.c
	gcc -w $^ -o client

#test_pseudos: test.c pseudos.c pseudos.h
#	gcc -w $^ -o $@

#sample_server.o: sample_server.c

threads_manager.c: threads_manager.h

pseudos.c: pseudos.h

players.c: players.h pseudos.h

card.c: card.h

client_game.c: client_game.h card.h

PHONY:clean

clean:
	@rm *~ *.o ./server ./client ./test_pseudos
