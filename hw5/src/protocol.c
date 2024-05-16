#include "Includes.h"
#include "protocol.h"

int customWrite(int fd, const void* buf, size_t count) 
{
    if (count <= 0)
    {
        return 0;
    }
    size_t written = 0;
    while (written < count) 
    {
        ssize_t result = write(fd, buf + written, count - written);
        if (result == -1) 
        {
            return -1; 
        }
        written = written + result;
    }
    return 0;
}

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload) 
{
    if (hdr == NULL) 
    {
        errno = EINVAL;
        return -1;
    }
    if (customWrite(fd, hdr, sizeof(CHLA_PACKET_HEADER)) == -1) 
    {
        return -1;
    }
    if (payload != NULL && customWrite(fd, payload, ntohl(hdr->payload_length)) == -1)
    {
        return -1;
    }
    return 0;
}

ssize_t customRead(int fd, void *buf, size_t count) 
{
    if (buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    size_t readed = 0;
    while (readed < count)
    {
        ssize_t result = read(fd, buf + readed, count - readed);
        switch(result)
        {
            case 0:
                return readed != count ? -1 : 0;
            case -1:
                return -1;
            default:
                readed = readed + result;
        }
    }
    return readed != count ? -1 : 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload) 
{
    if (hdr == NULL || payload == NULL) 
    {
        errno = EINVAL;
        return -1;
    }
    if (customRead(fd, hdr, sizeof(CHLA_PACKET_HEADER)) == -1) 
    {
        return -1;
    }
    void* temp = *payload;
    uint32_t payload_length = ntohl(hdr->payload_length);  
    if (payload_length)
    {
        if (customRead(fd, *payload = calloc(1, payload_length), payload_length) == -1) 
        {
            free(*payload); 
            *payload = temp;
            return -1;
        }
    }
    return 0;
}