BOOST_PATH = ${BOOST_DIR}
CXX        = g++
CFLAGS     = -Wall -O2 -std=c++0x -Wno-sign-compare
LDFLAGS    =
INCLUDES   = -I${BOOST_PATH}/include/
LIBS       = -L${BOOST_PATH}/lib -lboost_regex -lboost_program_options
TARGET     = qm
OBJS       = src/main.o src/quine_mccluskey.o

all:	$(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) *~ \#*

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

