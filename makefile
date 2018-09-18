CC = g++
CFLAGS = -Wall -std=c++1z -lssl -lcrypto -pthread
DEPS = create_sha.h client.h
OBJ = client.o create_sha.o uploader.o
%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
