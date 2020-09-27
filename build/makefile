VPATH = ../src/
target1 = ../bin/serverNew
target2 = ../bin/clientNew
midObj1 = $(addprefix $(VPATH),serverNew.o)
midObj2 = $(addprefix $(VPATH),clientNew.o)
cpp1 = serverNew.cpp
cpp2 = clientNew.cpp
GG = g++ -std=c++11

target : $(target1) $(target2)

$(target1): $(midObj1)
	$(GG) -pthread -o $(target1) $(midObj1)
$(target2): $(midObj2)
	$(GG) -pthread -o $(target2) $(midObj2)
$(midObj1) $(midObj2): %.o:%.cpp
	$(GG) -c $< -o $@
clean:
	
