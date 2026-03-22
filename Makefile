CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -O2 -Iinclude

SRC = \
	src/tlc_runtime.c \
	src/tlc_string.c \
	src/tlc_json.c \
	src/tlc_message.c \
	src/tlc_memory.c \
	src/tlc_prompt.c \
	src/tlc_parser.c \
	src/tlc_model.c \
	src/tlc_tool.c \
	src/tlc_chain.c \
	src/tlc_agent.c \
	src/tlc_vectorstore.c \
	src/tlc_retriever.c \
	src/tlc_embeddings.c

OBJ = $(SRC:.c=.o)

all: example_mock example_posix

libtidylangchain.a: $(OBJ)
	ar rcs $@ $(OBJ)

example_mock: libtidylangchain.a examples/example_mock_transport.c
	$(CC) $(CFLAGS) -o $@ examples/example_mock_transport.c libtidylangchain.a

example_posix: libtidylangchain.a examples/example_posix_curl_transport.c
	$(CC) $(CFLAGS) -o $@ examples/example_posix_curl_transport.c libtidylangchain.a

clean:
	rm -f $(OBJ) libtidylangchain.a example_mock example_posix
