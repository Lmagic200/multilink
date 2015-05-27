#include "transport.h"
#include "packet_stream.h"
#include "packet_stream_util.h"
#include "misc.h"
#include "exfunctional.h"
#include "logging.h"

std::vector<std::shared_ptr<PacketStream> > packet_stream_pair(Reactor& reactor) {
    auto p = fd_pair(reactor);
    return mapvector(p, [&](FD* fd) -> std::shared_ptr<PacketStream> {
        return LengthPacketStream::create(reactor, fd);
    });
}

void packet_printer(Reactor& reactor, std::shared_ptr<PacketStream> stream, std::string name) {
    auto on_recv_ready = [name, stream]() {
        while(true) {
            auto packet = stream->recv();
            if(!packet) break;
            LOG(name << " recv " << *packet);
        }
    };
    stream->set_on_recv_ready(on_recv_ready);
    reactor.schedule(on_recv_ready);
}

int main() {
    Reactor reactor;

    std::vector<std::shared_ptr<PacketStream> > network = packet_stream_pair(reactor);
    std::vector<std::shared_ptr<PacketStream> > target = packet_stream_pair(reactor);

    TargetCreator creator = [&](uint64_t id) -> Future<std::shared_ptr<PacketStream> > {
        assert(id == 0);
        LOG("create");
        return make_future(target[1]);
    };
    auto transport = Transport::create(reactor, network[1], creator, 4096);

    target[1]->set_on_send_ready(nothing);
    target[1]->set_on_recv_ready(nothing);

    target[0]->set_on_send_ready(nothing);
    network[0]->set_on_send_ready(nothing);
    packet_printer(reactor, network[0], "network");
    packet_printer(reactor, target[0], "target");

    AllocBuffer ab(17);
    Buffer b = ab.as_buffer();
    b.convert<uint64_t>(0) = 0;
    b.convert<uint64_t>(8) = 0;
    b.data[16] = 'z';
    network[0]->send(b);

    reactor.run();
}
