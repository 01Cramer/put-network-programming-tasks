#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

 // compile with -lpcap

 #define TCP 6
 #define UDP 17
 
 char* errbuf;
 pcap_t* handle;

 int packets [4] = 
 {
     0, // ARP
     0, // IP
     0, // IP/UDP
     0  // IP/TCP
 };
 
 void cleanup() {
   pcap_close(handle);
   free(errbuf);
 }
 
 void stop(int signo) {
   printf("\n--- Statistics ---\n");
   printf("ARP: %d\n", packets[0]);
   printf("IP: %d\n", packets[1]);
   printf("TCP: %d\n", packets[2]);
   printf("UDP: %d\n", packets[3]);
   exit(EXIT_SUCCESS);
 }
 
 void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
    struct ethhdr* eth = (struct ethhdr*) bytes;
    unsigned short eth_type = ntohs(eth->h_proto);

    switch(eth_type)
    {
      case ETH_P_ARP:
        packets[0]++;
        break;
      case ETH_P_IP:
        packets[1]++;

        struct iphdr* iph = (struct iphdr*) (bytes + sizeof(struct ethhdr)); // sizeof(struct ethhdr) is 14 bytes
        unsigned short protocol = iph->protocol;
        switch(protocol)
        {
          case TCP:
            packets[2]++;
            break;
          case UDP:
            packets[3]++;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
 }
 
 int main(int argc, char** argv) {
  atexit(cleanup);
  signal(SIGINT, stop);
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_set_timeout(handle, 1000);
  pcap_activate(handle);
  pcap_loop(handle, -1, trap, NULL);
}