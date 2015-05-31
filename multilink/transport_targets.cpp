#include "transport_targets.h"
#include "packet_stream_util.h"
#define LOG_NAME "transport_targets"
#include "logging.h"

TargetCreator create_connecting_tcp_target_creator(Reactor& reactor, std::string host, int port) {
    return [host, port, &reactor](uint64_t id) -> Future<std::shared_ptr<PacketStream> > {
        return TCP::connect(reactor, host, port).then<std::shared_ptr<PacketStream> >([&reactor](FD* fd) -> std::shared_ptr<PacketStream> {
            return FreePacketStream::create(reactor, fd);
        });
    };
}

void create_listening_tcp_target_creator(Reactor& reactor, std::shared_ptr<Transport> transport, std::string host, int port) {
    std::shared_ptr<uint64_t> next_id = std::make_shared<uint64_t>(0);

    TCP::listen(reactor, host, port, [&reactor, next_id, transport] (FD* fd) {
        uint64_t& id_ref = *(next_id);
        uint64_t id = id_ref++;

        auto stream = FreePacketStream::create(reactor, fd);
        transport->add_target(id, Future<std::shared_ptr<PacketStream> >::make_immediate(stream));
    });
}

TargetCreator unknown_stream_target_creator() {
    return [](uint64_t) -> Future<std::shared_ptr<PacketStream> > {
        // TODO: report error
        LOG("use of unknown stream");
        return make_future(std::shared_ptr<PacketStream> {});
    };
}
