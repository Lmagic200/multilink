#include "multilink.h"

namespace Multilink {
    Multilink::Multilink() {

    }

    void Multilink::send(ChannelInfo channel,
                         const char* buff, size_t size) {

    }

    ssize_t Multilink::recv(ChannelInfo& channel,
                 const char* buff, size_t size) {
        return -1;
    }

    Link& Multilink::add_link(Stream* stream, std::string name) {
        return *((Link*)NULL);
    }
}
