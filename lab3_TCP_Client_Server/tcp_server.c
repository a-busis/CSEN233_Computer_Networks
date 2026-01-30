#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

//Define the number of clients/threads that can be served
#define N 100
int threadCount = 0;
pthread_t clients[N]; //declaring N threads

 //Declare socket and connection file descriptors.
int sockfd;
int connfd;

 //Declare server address to which to bind for receiving messages and client address to fill in sending address
struct sockaddr_in servAddr;
struct sockaddr_in clienAddr;
socklen_t sin_size;

//Connection handler (thread function) for servicing client requests for file transfer
void* connectionHandler(void* sock){
  int thisconnfd = *(int*)sock;
  int filefd;
  //declare buffer holding the name of the file from client
  char fnamebuf[1024];
  ssize_t fnamelen;
  char sbuf[1024];
  size_t sblen;

  //Connection established, server begins to read and write to the connecting client
  printf("Connection Established with client IP: %s and Port: %d\n", inet_ntoa(clienAddr.sin_addr), ntohs(clienAddr.sin_port));

  //receive name of the file from the client
  if ((fnamelen = recv(thisconnfd, fnamebuf, 1024, 0)) <= 0) {
    perror("Failure to receive filename from client");
    exit(0);
  }
  fnamebuf[fnamelen] = '\0';

  //open file and send to client
  printf("Client requested local file: %s\n", fnamebuf);
  filefd = open(fnamebuf, O_RDONLY);
  while((sblen = read(filefd, sbuf, 1024)) > 0) {
    send(thisconnfd, sbuf, sblen, 0);
  }

  //read file and send to connection descriptor

  printf("File transfer complete\n");

  //close file
  close(filefd);

  //Close connection descriptor
  close(thisconnfd);

  pthread_exit(0);
}


int main(int argc, char *argv[]){
  //Get from the command line, server IP, src and dst files.
  if (argc != 2){
    printf ("Usage: %s <port #> \n",argv[0]);
    exit(0);
  } 
  //Open a TCP socket, if successful, returns a descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  //Setup the server address to bind using socket addressing structure
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(atoi(argv[1]));
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //bind IP address and port for server endpoint socket
  bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr));


  // Server listening to the socket endpoint, and can queue 5 client requests
  printf("Server listening/waiting for client at port %s\n", argv[1]);
  listen(sockfd, N);

  while (1){
    //Server accepts the connection and call the connection handler
    sin_size = sizeof(struct sockaddr);
    connfd = accept(sockfd, (struct sockaddr*)&clienAddr, &sin_size);

    if(pthread_create(&clients[threadCount], NULL, connectionHandler, (void*) &connfd) < 0){
      perror("Unable to create a thread");
      exit(0);
    }
    else 
      printf("Thread %d has been created to service client request\n",++threadCount);
  }
  for(int i = 0; i < threadCount; i++){
    pthread_join(clients[i], NULL);
  }
  return 0;
}
