VPATH = ../src/
target1 = ../bin/serverNew
target2 = ../bin/clientNew
target3 = ../bin/Test
midObj1 = $(addprefix $(VPATH),serverNew.o)
midObj2 = $(addprefix $(VPATH),clientNew.o)
midObj3 = $(addprefix $(VPATH),Alloctor.o)
midObj4 = $(addprefix $(VPATH),Test.o)
cpp1 = serverNew.cpp
cpp2 = clientNew.cpp
GG = g++ -std=c++11 -g
CFLAG = 

target : $(target1) $(target2) $(target3)

$(target1): $(midObj1) $(midObj3)
	$(GG) -pthread -o $(target1) $(midObj1) $(midObj3)
$(target2): $(midObj2)
	$(GG) -pthread -o $(target2) $(midObj2)
$(target3): $(midObj4)
	$(GG) -pthread -o $(target3) $(midObj4)
$(midObj1) $(midObj2) $(midObj3) $(midObj4): %.o:%.cpp
	$(GG) $(CFLAG) -c $< -o $@
clean:
	
