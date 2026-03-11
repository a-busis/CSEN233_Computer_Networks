//UDP Server - Side (with rdt3.0)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PLOSTMSG 5
typedef struct {
  int seq_ack;
  int len;
  int cksum;
} Header;
typedef struct {
  Header header;
  char data[10];
} Packet;



//getChecksum()
int getChecksum(Packet packet) {
  packet.header.cksum = 0;
  int checksum = 0;
  char *ptr = (char *)&packet;
  char *end = ptr + sizeof(Header) + packet.header.len;
  while (ptr < end) {
      checksum ^= *ptr++;
  }
  return checksum;
}

//print packet
void printPacket(Packet packet) {
   printf("Packet{ header: { seq_ack: %d, len: %d, cksum: %d }, data: \"",
   packet.header.seq_ack,
   packet.header.len,
   packet.header.cksum);
   fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
   printf("\" }\n");
}

//serverSend()
void serverSend(int sockfd, const struct sockaddr *address, socklen_t addrlen, int seqnum) {
  // Simulating a chance that ACK gets lost
  if (rand() % PLOSTMSG == 0) {
     printf("Dropping ACK\n");
  }
  else{
    Packet packet;
    //prepare and send the ACK
    packet.header.seq_ack = seqnum;
    packet.header.len = 0;
    packet.header.cksum = getChecksum(packet);
    sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);

    printf("Sent ACK %d, checksum %d\n", packet.header.seq_ack, packet.header.cksum);
  }
}

Packet serverReceive(int sockfd, int seqnum) {
  Packet packet;
  struct sockaddr clientAddr;
  socklen_t clientAddrLen;
  while (1) {
    //Receive a packet from the client
    recvfrom(sockfd, &packet, sizeof(Packet), 0, &clientAddr, &clientAddrLen);
    // validate the length of the packet
    // print what was received
    printf("Received: ");
    printPacket(packet);
    //verify the checksum and the sequence number
    if (packet.header.cksum != getChecksum(packet)) {
      printf("Bad checksum, expected %d\n", getChecksum(packet));
      serverSend(sockfd, &clientAddr, clientAddrLen, (seqnum + 1) % 2);
    } else if (packet.header.seq_ack != seqnum) {
      printf("Bad seqnum, expected %d\n", seqnum);
      serverSend(sockfd, &clientAddr, clientAddrLen, (seqnum + 1) % 2);
    } else {
      printf("Good packet\n");
      serverSend(sockfd, &clientAddr, clientAddrLen, seqnum);
      break;
    }
  }
  printf("\n");
  return packet;
}

int main(int argc, char *argv[]) {
  // check arguments
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <port> <outfile>\n", argv[0]);
    exit(1);
  }
  // seed the RNG
  srand((unsigned)time(NULL));
  // create a socket
  int sockfd;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  // initialize the server address structure
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(atoi(argv[1]));
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind the socket
  bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  printf("Server socket bound to port %s, listening for packets\n", argv[1]);

  //open file using argv[2]
  int fp = open(argv[2],O_CREAT | O_RDWR,0666);
  if (fp < 0){
    perror("file failed to open\n");
    exit(1);
  }


  // get file contents from client and save it to the file
  int seqnum = 0;
  Packet packet;
  do {
    packet = serverReceive(sockfd, seqnum);
    write(fp, packet.data, packet.header.len);
    seqnum = (seqnum + 1) % 2;
  } while (packet.header.len > 0);
  //cleanup
  close(sockfd);
  return 0;
}
