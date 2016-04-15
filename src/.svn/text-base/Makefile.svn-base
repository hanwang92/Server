# The storage client library.
CLIENTLIB = libstorage.a

# The programs to build.
TARGETS = $(CLIENTLIB) yaccer lexer server client encrypt_passwd

# The source files.
SRCS = server.c storage.c utils.c client.c encrypt_passwd.c database.c config_parser.y config_parser.l config_parser.tab.c

# Compile flags.
CFLAGS = -g -Wall
LDFLAGS = -g -Wall -lcrypt

CC=    gcc
CFLAGS= -g
YYFLAGS= -v  -d
YACC=bison
LEX=flex
INCLUDE = -I.

# Dependencies file
DEPEND_FILE = depend.mk


# ---------------------------------------------------------
# Targets to build the storage client and server.
# ---------------------------------------------------------

# Default targets.
build: $(TARGETS)

yaccer: config_parser.y utils.c
	${YACC} -d config_parser.y 
	${CC} ${CFLAGS} ${INCLUDE} -c config_parser.tab.c utils.c

lexer:	config_parser.l utils.c
	${LEX} config_parser.l 
	${CC} ${CFLAGS} ${INCLUDE} -c lex.yy.c utils.c

#parser: config_parser.tab.o lex.yy.o
#	${CC} ${CFLAGS} ${INCLUDE} -c parser.c
#	${CC} -o p ${CFLAGS} ${INCLUDE} lex.yy.o config_parser.tab.o parser.o .c


# Build the client library.
$(CLIENTLIB): storage.o utils.o 
	$(AR) rcs $@ $^

# Build the server.
server: server.o utils.o database.o lex.yy.o config_parser.tab.o
	${YACC} -d config_parser.y
	${CC} ${CFLAGS} ${INCLUDE} -c config_parser.tab.c
	${LEX} config_parser.l
	${CC} ${CFLAGS} ${INCLUDE} -c lex.yy.c
	${CC} ${CFLAGS} ${INCLUDE} -c config_parser.tab.c
	${CC} ${CFLAGS} ${INCLUDE} -c server.c
	$(CC) $(LDFLAGS) $^ -o $@

# Build the client.
client: client.o $(CLIENTLIB)
	$(CC) $(LDFLAGS) $^ -o $@

# Build the password encryptor.
encrypt_passwd: encrypt_passwd.o utils.o
	$(CC) $(LDFLAGS) $^ -o $@




# Compile a .c source file to a .o object file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
# Delete generated files.
clean:
	-rm -rf $(TARGETS) *.o tags $(DEPEND_FILE)
	rm -f p config_parser.tab.h config_parser.tab.c parser.o \
	   config_parser.tab.o lex.yy.c lex.yy.o *~ \

# Create dependencies file.
depend:
	$(CC) $(CFLAGS) -M $(SRCS) > $(DEPEND_FILE)

-include $(DEPEND_FILE)


# ---------------------------------------------------------
# Targets to help work with the code.
# ---------------------------------------------------------

# Create tags file to help with code navigation in some editors.
tags: *.c *.h
	ctags -R .

# Create tags file including system header files.
fulltags: *.c *.h
	ctags -R . `$(MAKE) -s print_include_dirs`

# Print the include directories for current system.
print_include_dirs:
	@gcc -x c -Wp,-v -E - < /dev/null 2>&1 |grep '^ '

.PHONY: build depend clean tags fulltags print_include_dirs



