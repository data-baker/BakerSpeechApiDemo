CC			=	g++
SRCS		=	httpclient.cpp tts_longtext_example.cpp main.cpp
OBJS		=	*.o
LIBSPATH	=	-L../deps/lib -L../deps/json-cpp/lib -L../deps/curl/lib
RLIBSPATH	=	../deps/lib
EXLIBSPATH	=	../deps/lib
INC 		=	-I. -I./include  -I../deps/ -I../deps/json-cpp/include -I../deps/curl/include -I../deps/openssl/include

TARGET 	= 	tts_longtext_example

C_FLAGS 	=	-g -std=c++11 -fPIC -fno-inline -Wl,-rpath=$(EXLIBSPATH) -Wl,-rpath-link=$(RLIBSPATH)
LIBS 		=	-lcurl -lcrypto -lssl -ljson_libmt -lpthread
				
${TARGET}: ${SRCS}
	$(CC) $(SRCS) $(INC) $(C_FLAGS) -o $(TARGET) $(LIBSPATH) $(LIBS) 
clean:
	rm -f $(TARGET)
	rm -f *.o
