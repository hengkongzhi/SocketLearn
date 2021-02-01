VPATH = ./
target1 = ./main
midObj1 = $(addprefix $(VPATH),Alloctor.o)
midObj2 = $(addprefix $(VPATH),main.o)
GG = g++ -std=c++11 -g

target : $(target1)
$(target1): $(midObj1) $(midObj2)
	$(GG) $(midObj1) $(midObj2) -o $(target1)
$(midObj1) $(midObj2): %.o:%.cpp
	$(GG) -c $< -o $@
clean:
	
