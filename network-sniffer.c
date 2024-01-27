#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>      

void fPacketHandler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) 
{
    struct ether_header *sEthernetHeader = (struct ether_header *)packet;

    printf("MAC Addresses: ");
    for(int i = 0; i < 6; i++) 
    {
        printf("%02x", sEthernetHeader->ether_shost[i]);
        
        if (i < 5) 
        {
            printf(":");
        }
    }
    
    printf(" -> ");
    
    for(int i = 0; i < 6; i++) 
    {
        printf("%02x", sEthernetHeader->ether_dhost[i]);
        
        if (i < 5) 
        {
            printf(":");
        }
    }

    if (ntohs(sEthernetHeader->ether_type) == ETHERTYPE_IP)
     {
        const struct iphdr *sIpHeader = (struct iphdr *)(packet + sizeof(struct ether_header));

        struct in_addr sSourceIPAddr, sDestIPAddr;
        sSourceIPAddr.s_addr = sIpHeader->saddr;
        sDestIPAddr.s_addr = sIpHeader->daddr;

        printf(", IP Addresses: %s -> %s", inet_ntoa(sSourceIPAddr), inet_ntoa(sDestIPAddr));
    }

    printf("\n");
}

int main(int argc, char *argv[]) 
{
    pcap_if_t *pAllDevices, *pDevice;
    char aErrorBuffer[PCAP_ERRBUF_SIZE];
    pcap_t *pCapHandle;

    if (pcap_findalldevs(&pAllDevices, aErrorBuffer) == -1) {
        fprintf(stderr, "Error: Couldn't find the devices: %s\n", aErrorBuffer);
        return 1;
    }

    pDevice = pAllDevices;
    if (pDevice == NULL) {
        fprintf(stderr, "Error: Not found any devices\n");
        pcap_freealldevs(pAllDevices); 
        return 1;
    }

    printf("Using ethernet device: %s\n", pDevice->name);

    pCapHandle = pcap_open_live(pDevice->name, BUFSIZ, 1, 1000, aErrorBuffer);
    if (pCapHandle == NULL) {
        fprintf(stderr, "Error: Device can not be opened %s\n", aErrorBuffer);
        pcap_freealldevs(pAllDevices);
        return 1;
    }

    pcap_loop(pCapHandle, 0, fPacketHandler, NULL);

    pcap_close(pCapHandle);

    pcap_freealldevs(pAllDevices);

    return 0;
}

