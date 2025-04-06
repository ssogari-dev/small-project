#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "myheader.h"

void print_mac(const char *label, const u_char *mac) {
    printf("%s %02x:%02x:%02x:%02x:%02x:%02x\n", label,
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    // 1) Ethernet 헤더 위치 (패킷 시작)
    struct ethheader *eth = (struct ethheader *)packet;

    // Ethernet 출발지/목적지 MAC 주소 출력
    print_mac("Ethernet SRC MAC:", eth->ether_shost);
    print_mac("Ethernet DST MAC:", eth->ether_dhost);

    // 2) EtherType 확인: IP 패킷(0x0800)인지
    if (ntohs(eth->ether_type) == 0x0800) {
        // IP 헤더는 Ethernet 헤더 바로 다음 위치
        struct ipheader *ip = (struct ipheader *)(packet + sizeof(struct ethheader));
        unsigned int ip_header_len = ip->iph_ihl * 4; // IHL 필드: 32비트 워드 단위 길이

        // IP 출발지/목적지 주소 출력
        printf("IP SRC: %s\n", inet_ntoa(ip->iph_sourceip));
        printf("IP DST: %s\n", inet_ntoa(ip->iph_destip));

        // 3) IP 프로토콜이 TCP(6)인지 확인 후 TCP 헤더 파싱
        if (ip->iph_protocol == IPPROTO_TCP) {
            // TCP 헤더는 Ethernet + IP 헤더 뒤
            struct tcpheader *tcp = (struct tcpheader *)(packet + sizeof(struct ethheader) + ip_header_len);

            // TCP 출발지/목적지 포트 출력
            printf("TCP SRC Port: %u\n", ntohs(tcp->tcp_sport));
            printf("TCP DST Port: %u\n", ntohs(tcp->tcp_dport));
        }
    }
    printf("----------------------------------------\n");
}

int main() {
    char *dev = "enp0s3";                  // 네트워크 인터페이스
    char errbuf[PCAP_ERRBUF_SIZE];         // libpcap 오류 메시지 버퍼
    pcap_t *handle;                        // pcap 세션 핸들
    struct bpf_program fp;                 // 컴파일된 BPF 프로그램
    char filter_exp[] = "tcp";             // BPF 필터 표현식: TCP 패킷만 캡처
    bpf_u_int32 net = 0;                   // 네트워크 주소 (여기서는 사용하지 않음)

    // 1) 인터페이스 열기 (promiscuous 모드, 1초 타임아웃)
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "장치 %s 열기 실패: %s\n", dev, errbuf);
        return EXIT_FAILURE;
    }

    // 2) 필터 표현식을 BPF 바이트코드로 컴파일
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "필터 컴파일 오류: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        return EXIT_FAILURE;
    }
    // 컴파일된 필터를 캡처 세션에 적용
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "필터 적용 오류: %s\n", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        return EXIT_FAILURE;
    }

    // 3) 무한 캡처 루프 시작, got_packet 콜백 호출
    pcap_loop(handle, -1, got_packet, NULL);

    // 4) 정리: 필터 해제 및 세션 종료
    pcap_freecode(&fp);
    pcap_close(handle);
    return EXIT_SUCCESS;
}
