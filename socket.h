#ifndef SOCKET_H
#define SOCKET_H

class Socket {
};

class UdpSocket: public Socket {

private:
    const char *address;
    int port;
    int sock;

public:
    UdpSocket(const char *address, int &port);
    ~UdpSocket();

    int sendto(const char *buf, int len, const struct sockaddr_in *dest_addr) const;
    int recvfrom(char *buf, int len, struct sockaddr_in *src_addr) const;
};

#endif // SOCKET_H
