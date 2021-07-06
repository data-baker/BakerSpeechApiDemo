CC			=	g++
SRCS		=	tts_example.cpp
OBJS		=	*.o
LIBSPATH	=	-L./lib
RLIBSPATH	=	./lib
EXLIBSPATH	=  ./lib
INC 		=	-I. -I./include
LIBRARY 	= 	tts_example
C_FLAGS 	=	-g -std=c++11 -fPIC -fno-inline -Wl,-rpath=$(EXLIBSPATH) -Wl,-rpath-link=$(RLIBSPATH)
LIBS 		=	-lcurl\
				-lcrypto\
            -lssl\
				-ljson_libmt
				
${LIBRARY}: ${SRCS}
	$(CC) $(SRCS) $(INC) $(C_FLAGS) -o $(LIBRARY) $(LIBSPATH) $(LIBS) 
clean:
	rm $(LIBRARY)
