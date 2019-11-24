// C program to implement one side of FIFO 
// This side writes first, then reads 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <iostream>

using namespace std;


int main() 
{ 
	int fd; 
	const char * myfifo = "./myfifo"; 
	mkfifo(myfifo, 0666); 
	if((fd = open(myfifo, O_WRONLY)) < 0) 
		cout << "Error" << endl;
	string value = "TEST";
	if ( write(fd,value.c_str(), value.size()+1) != value.size()+1) { 
        close(fd);
        exit(1);
    } 
	return 0; 
} 
