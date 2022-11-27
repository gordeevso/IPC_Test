#pragma once

#include <climits>
#include <cstring>
#include <cstdint>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

#include "readerwriterqueue.h"

namespace Common {

struct Buffer {
    std::vector<std::uint8_t> data;
    std::size_t countBytes = 0;
};

using LockFreeSPSCQueueT = moodycamel::ReaderWriterQueue<Buffer*>;

class ThreadSafeQueueBuffer {
    std::vector<Buffer> buffers;
    LockFreeSPSCQueueT queueUsed;
    LockFreeSPSCQueueT queueInProcess;

    static Buffer& dequeue(LockFreeSPSCQueueT& queue);
    static void enqueue(LockFreeSPSCQueueT& queue, Buffer* buffer);

public:
    ThreadSafeQueueBuffer(std::size_t defaultBufferSize, std::size_t countBuffers);

    Buffer& dequeueReadyToUse() { return dequeue(queueUsed); }
    void enqueueUsed(Buffer* buffer) { enqueue(queueUsed, buffer); }

    Buffer& dequeueInProcess() { return dequeue(queueInProcess); }
    void enqueueInProcess(Buffer* buffer) { enqueue(queueInProcess, buffer); }

    std::size_t countToUse() const { return queueUsed.size_approx(); }
    std::size_t capacityToUse() const { return queueUsed.max_capacity(); }
    std::size_t countInProcess() const { return queueInProcess.size_approx(); }
    std::size_t capacityInProcess() const { return queueInProcess.max_capacity(); }
};

class NamedPipe {
    std::string m_pipePath;

public:
    explicit NamedPipe(std::string_view pipePath);
    ~NamedPipe();

    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe& other) = delete;
    NamedPipe(NamedPipe&& other) noexcept;
    NamedPipe& operator=(NamedPipe&& other) noexcept;

    void write(const std::vector<std::uint8_t>& bufferToWrite);
    void read(Buffer& buffer);
};

template<typename T>
void checkErrors(const T value, const T badValue) {
    if(value == badValue) {
        throw std::system_error(errno, std::generic_category());
    }
}

#if NET_DEBUG
    #define NET_ASSERT(condition) \
    assert((condition))
    #define NET_CHECK(value, badValue) \
    Common::checkErrors((value), (badValue))
#else
    #define NET_ASSERT(condition)
    #define NET_CHECK(value, badValue)
#endif

enum class ProtocolType {
    TCP,
    UDP
};

template<ProtocolType Protocol>
class NetworkReaderWriter {
    int m_socketFileDescriptor = 0;

    static sockaddr_in getAddressStructHelper(std::uint16_t port);

    static int connect(int socketDescriptor, std::uint16_t port, std::string_view ipv4);
    static int bind(int socketDescriptor, std::uint16_t port);

public:
    explicit NetworkReaderWriter(std::uint16_t port);
    NetworkReaderWriter(std::uint16_t port, std::string_view ipv4Address);

    NetworkReaderWriter(const NetworkReaderWriter&) = delete;
    NetworkReaderWriter& operator=(const NetworkReaderWriter&) = delete;
    NetworkReaderWriter(NetworkReaderWriter&& other) noexcept;
    NetworkReaderWriter& operator=(NetworkReaderWriter&& other) noexcept;

    ~NetworkReaderWriter();

    int bind(std::uint16_t port) const;
    int reConnect(std::uint16_t port, std::string_view ipv4);

    std::int64_t read(std::vector<std::uint8_t>& bufferToRead) const;
    std::int64_t write(std::vector<std::uint8_t>& dataToSend) const;
};

template<ProtocolType Protocol>
sockaddr_in NetworkReaderWriter<Protocol>::getAddressStructHelper(std::uint16_t port) {
    sockaddr_in serverAddress{};
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddress.sin_port = ::htons(port);
    return serverAddress;
}

template<>
int NetworkReaderWriter<ProtocolType::UDP>::bind(int socketDescriptor, std::uint16_t port) {
    sockaddr_in serverAddress = getAddressStructHelper(port);
    const int result = ::bind(socketDescriptor, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    NET_CHECK(result, -1);
    return result;
}

template<>
int NetworkReaderWriter<ProtocolType::TCP>::connect(int socketDescriptor, std::uint16_t port, std::string_view ipv4) {
    sockaddr_in serverAddress = getAddressStructHelper(port);
    const int resultInetPton = ::inet_pton(AF_INET, ipv4.data(), &serverAddress.sin_addr);
    NET_CHECK(resultInetPton, -1);
    const int resultConnect = ::connect(socketDescriptor, reinterpret_cast<sockaddr*>(&serverAddress),sizeof(serverAddress));
    return resultConnect;
}

template<ProtocolType Protocol>
NetworkReaderWriter<Protocol>::NetworkReaderWriter(NetworkReaderWriter&& other) noexcept {
    *this = std::move(other);
}

template<ProtocolType Protocol>
NetworkReaderWriter<Protocol>& NetworkReaderWriter<Protocol>::operator=(NetworkReaderWriter&& other) noexcept {
    if(this != &other) {
        m_socketFileDescriptor = other.m_socketFileDescriptor;
        other.m_socketFileDescriptor = 0;
    }
    return *this;
}

template<ProtocolType Protocol>
NetworkReaderWriter<Protocol>::~NetworkReaderWriter() {
    ::close(m_socketFileDescriptor);
}

template<>
int NetworkReaderWriter<ProtocolType::UDP>::bind(std::uint16_t port) const {
    return NetworkReaderWriter::bind(m_socketFileDescriptor, port);
}

template<>
int NetworkReaderWriter<ProtocolType::TCP>::reConnect(std::uint16_t port, std::string_view ipv4) {
    close(m_socketFileDescriptor);
    const int socketDescriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    NET_CHECK(socketDescriptor, -1);
    const int result = NetworkReaderWriter::connect(socketDescriptor, port, ipv4);
    m_socketFileDescriptor = socketDescriptor;
    return result;
}

template<ProtocolType Protocol>
std::int64_t NetworkReaderWriter<Protocol>::read(std::vector<std::uint8_t>& bufferToRead) const {
    const ssize_t readBytes = ::read(m_socketFileDescriptor, bufferToRead.data(), bufferToRead.size());
    return readBytes;
}

template<ProtocolType Protocol>
std::int64_t NetworkReaderWriter<Protocol>::write(std::vector<std::uint8_t>& dataToSend) const {
    const ssize_t result = ::send(m_socketFileDescriptor, dataToSend.data(), dataToSend.size(), MSG_NOSIGNAL);
    return result;
}

template<>
NetworkReaderWriter<ProtocolType::UDP>::NetworkReaderWriter(std::uint16_t port) {
    const int socketDescriptor = ::socket(AF_INET, SOCK_DGRAM, 0);
    NET_CHECK(socketDescriptor, -1);
    NetworkReaderWriter::bind(socketDescriptor, port);
    m_socketFileDescriptor = socketDescriptor;
}

template<>
NetworkReaderWriter<ProtocolType::TCP>::NetworkReaderWriter(std::uint16_t port, std::string_view ipv4Address) {
    reConnect(port, ipv4Address);
}

} // namespace Common