OUTPUT = a.exe

LINK =
INCLUDE = 
CS = -Wall -Wextra -std=c++26 -Wno-unused-parameter -Wno-sign-compare -Wno-shift-op-parentheses -Wno-invalid-offsetof

debug:
	g++ $(CS) -g src/main.cpp -o $(OUTPUT) $(INCLUDE) $(LINK)

release:
	g++ $(CS) -s -O3 src/main.cpp -o $(OUTPUT) $(INCLUDE) $(LINK)

clean:
	rm $(OUTPUT)
