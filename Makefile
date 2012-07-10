CFLAGS    = -Wall -O2 -std=c++0x 
LDFLAGS   =
INCLUDES  = -I${BOOST_DIR}/include/
LIBS      = -L${BOOST_DIR}/lib -lboost_regex
TARGET    = qm
OBJS      = src/main.o

all:	$(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) *~ \#*

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
