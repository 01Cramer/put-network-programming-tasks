#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  socklen_t sl;
  int sfd, s_sfd, cfd, on = 1;
  struct sockaddr_in saddr, second_saddr, caddr;
  struct hostent* addrent;
  
  char buf[512];
  int rc;
  
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(1234);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  
  addrent = gethostbyname(argv[1]);
  memset(&second_saddr, 0, sizeof(second_saddr));
  second_saddr.sin_family = AF_INET;
  second_saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&second_saddr.sin_addr.s_addr, addrent->h_addr_list[0], addrent->h_length);
  s_sfd = socket(PF_INET, SOCK_STREAM, 0);
  
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
  bind(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  listen(sfd, 5);
  while(1) {
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl);
    rc = recv(cfd, buf, 512, 0);
    close(cfd);
    
    connect(s_sfd, (struct sockaddr*) &second_saddr, sizeof(second_saddr));
    send(s_sfd, buf, rc, 0);
    close(s_sfd);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
