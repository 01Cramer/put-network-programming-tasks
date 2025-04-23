 #include <pcap.h>
 #include <signal.h> 
 #include <libnet.h>
 #include <stdlib.h>
 #include <string.h>

// compile with -lpcap -lnet

 struct arphdr
 {
    u_int16_t ftype;
    u_int16_t ptype;
    u_int8_t  flen;
    u_int8_t  plen;
    u_int16_t opcode;
    u_int8_t  sender_mac_addr[6];
    u_int8_t  sender_ip_addr[4];
    u_int8_t  target_mac_addr[6];
    u_int8_t  target_ip_addr[4];
 };

u_int32_t target_ip_addr, src_ip_addr;

char* errbuf;
pcap_t* handle;

void cleanup() {
  pcap_close(handle);
  free(errbuf);
}

void stop(int signo) {
  exit(EXIT_SUCCESS);
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
    struct arphdr* arp = (struct arphdr*)(bytes + 14);
    if(ntohs(arp->opcode) == 2 && memcmp(arp->sender_ip_addr, &target_ip_addr, 4) == 0)
    {
      printf("Reply from: ");
      
      printf(" %d", arp->sender_ip_addr[0]);
      printf(" %d", arp->sender_ip_addr[1]);
      printf(" %d", arp->sender_ip_addr[2]);
      printf(" %d ", arp->sender_ip_addr[3]);

      printf("[");
      printf(" %02x", arp->sender_mac_addr[0]);
      printf(" %02x", arp->sender_mac_addr[1]);
      printf(" %02x", arp->sender_mac_addr[2]);
      printf(" %02x", arp->sender_mac_addr[3]);
      printf(" %02x", arp->sender_mac_addr[4]);
      printf(" %02x", arp->sender_mac_addr[5]);
      printf(" ]\n");
    }
}


 
 int main(int argc, char** argv) {
   libnet_t *ln;
   u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
            zero_hw_addr[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   struct libnet_ether_addr* src_hw_addr;
   char errbuf_2[LIBNET_ERRBUF_SIZE];
 
   ln = libnet_init(LIBNET_LINK, argv[1], errbuf_2);
   src_ip_addr = libnet_get_ipaddr4(ln);
   src_hw_addr = libnet_get_hwaddr(ln);
   target_ip_addr = libnet_name2addr4(ln, argv[2], LIBNET_RESOLVE);
   libnet_autobuild_arp(
     ARPOP_REQUEST,                   /* operation type       */
     src_hw_addr->ether_addr_octet,   /* sender hardware addr */
     (u_int8_t*) &src_ip_addr,        /* sender protocol addr */
     zero_hw_addr,                    /* target hardware addr */
     (u_int8_t*) &target_ip_addr,     /* target protocol addr */
     ln);                             /* libnet context       */
   libnet_autobuild_ethernet(
     bcast_hw_addr,                   /* ethernet destination */
     ETHERTYPE_ARP,                   /* ethertype            */
     ln);                             /* libnet context       */
     atexit(cleanup);
    signal(SIGINT, stop);
    errbuf = malloc(PCAP_ERRBUF_SIZE);
    handle = pcap_create(argv[1], errbuf);
    pcap_set_promisc(handle, 1);
    pcap_set_snaplen(handle, 65535);
    pcap_set_timeout(handle, 1000);
    pcap_activate(handle);
   while(1)
   {
    libnet_write(ln);
    pcap_loop(handle, 1, trap, NULL);
   }
   libnet_destroy(ln);
   return EXIT_SUCCESS;
 }
