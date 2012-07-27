BOOST_PATH = ${BOOST_DIR}
CFLAGS     = -Wall -O2 -std=c++0x -Wno-sign-compare
LDFLAGS    =
INCLUDES   = -I${BOOST_PATH}/include/
LIBS       = -L${BOOST_PATH}/lib -lboost_regex -lboost_program_options
TARGET     = qm
OBJS       = src/main.o

all:	$(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) *~ \#*

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
