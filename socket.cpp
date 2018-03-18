#include "socket.h"
#include "exception.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include <cassert>
#include <cstring>
#include <unistd.h>



UdpSocket::UdpSocket(const char *address, int &port) : address(address), port(port), sock(-1) {

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1) {
        throw Exception("Unable to create socket");
    }

    struct sockaddr_in addr;

    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(address == NULL) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if(inet_pton(AF_INET, address, &addr.sin_addr) != 1) {
            throw Exception("Invalid address format");
        }
    }

    if(bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        throw Exception("Unable to bind socket");
    }

    if(port == 0) {
        // get the assigned port number
        socklen_t len = sizeof(addr);
        if(getsockname(sock, (struct sockaddr *)&addr, &len) == -1) {
            throw Exception("Unable to get assigned port number");
        }
        port = ntohs(addr.sin_port);
    }

}

UdpSocket::~UdpSocket() {
    if(sock > 0) {
        close(sock);
    }
}

int UdpSocket::sendto(const char *buf, int len, const struct sockaddr_in *dest_addr) const {

    return (int)::sendto(sock, buf, len, 0,
        (const struct sockaddr *)dest_addr, sizeof(struct sockaddr_in));
}

int UdpSocket::recvfrom(char *buf, int len, struct sockaddr_in *src_addr) const {
    socklen_t addrlen;
    int bytes = (int)::recvfrom(sock, buf, len, 0, (struct sockaddr *)src_addr, &addrlen);
    if(bytes < 0){
      perror("recvfrom failed\n");
      close(sock);
    }
    return bytes;
}
