#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>


 //Declare socket file descriptor.
int sockfd;
int filefd;

char rbuf[1024];
int rb;

 //Declare server address to connect to
struct sockaddr_in servAddr;

int main(int argc, char *argv[]){
  //Get from the command line, server IP, src and dst files.
  if (argc != 5){
    printf ("Usage: %s <IP address> <port #> <source-file> <dest-file>\n",argv[0]);
    exit(0);
  } 
  //Open a TCP socket, if successful, returns a descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  //Setup the server address to connect to
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(atoi(argv[2]));
  servAddr.sin_addr.s_addr = inet_addr(argv[1]);

  //Connect the socket to the server
  if (connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
    perror("Failure to establish connection to server");
    exit(1);
  }
  printf("Connected to server at IP address %s and port %s\n", argv[1], argv[2]);

  //Send requested filename to server
  printf("Requesting file from server: %s\n", argv[3]);
  send(sockfd, argv[3], strlen(argv[3]), 0);

  //Open destination file to copy to
  printf("Writing data to destination file: %s\n", argv[4]);
  filefd = open(argv[4], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

  while ((rb = recv(sockfd, rbuf, 1024, 0)) > 0) {
    write(filefd, rbuf, rb);
  }

  close(filefd);
  close(sockfd);
  return 0;
}
