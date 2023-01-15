all:
	g++ -Wall -D_GLIBCXX_USE_CXX11_ABI=1 -c easywsclient.cpp easywsclient.hpp jsmn.hpp
	g++ -Wall -D_GLIBCXX_USE_CXX11_ABI=1 -c cCamillaDSP.cpp cCamillaDSP.hpp
	g++ -Wall -D_GLIBCXX_USE_CXX11_ABI=1 -o testcdsp Main.cpp cCamillaDSP.o easywsclient.o
clean:
	-rm -f *.o
	-rm -f *.gch
	-rm -f cCamillaDSP
	-rm -f testcdsp
