#include "Common.h"

namespace Common {

Buffer& ThreadSafeQueueBuffer::dequeue(LockFreeSPSCQueueT& queue) {
    Buffer* nextBuffer = nullptr;
    while(!queue.try_dequeue(nextBuffer)) {
        std::this_thread::yield();
    }
    return *nextBuffer;
}

void ThreadSafeQueueBuffer::enqueue(LockFreeSPSCQueueT& queue, Buffer* buffer) {
    // We don't want to allocate
    while(!queue.try_enqueue(buffer)) {
        std::this_thread::yield();
    }
}

ThreadSafeQueueBuffer::ThreadSafeQueueBuffer(std::size_t defaultBufferSize, std::size_t countBuffers)
    : buffers(countBuffers),
    queueUsed(countBuffers),
    queueInProcess(countBuffers) {
    // This number could be increased to make process ([msg with Entries] -> [A] -> [B] <-> [Server])
    // more efficient if we have small non-interleaving delays from 1st or 4th component, assuming A and B runs on same machine
    for(Buffer& buffer : buffers) {
        buffer.data.resize(defaultBufferSize);
        const bool result = queueUsed.try_enqueue(&buffer);
        NET_ASSERT(result);
    }
}




NamedPipe::NamedPipe(std::string_view pipePath) {
    // Ignore result since we don't know which process starts first
    ::mkfifo(pipePath.data(), 0666);
    m_pipePath = pipePath;
}

NamedPipe::~NamedPipe() {
    ::unlink(m_pipePath.c_str());
}

NamedPipe::NamedPipe(NamedPipe&& other) noexcept {
    *this = std::move(other);
}

NamedPipe& NamedPipe::operator=(NamedPipe&& other) noexcept {
    if(this != &other) {
        m_pipePath = other.m_pipePath;
        other.m_pipePath.clear();
    }
    return *this;
}

void NamedPipe::write(const std::vector<std::uint8_t>& bufferToWrite) {
    const std::int32_t countWrites = (static_cast<std::int32_t>(bufferToWrite.size()) / PIPE_BUF) + 1;
    std::int32_t offset = 0;
    // since max buffer size of pipe might be less than bufferToWrite, we might need
    // to do write in several iterations
    for(std::int32_t i = 0; i < countWrites; ++i) {
        const int fileDescriptor = ::open(m_pipePath.c_str(), O_WRONLY);
        NET_CHECK(fileDescriptor, -1);
        const std::size_t countBytesToSend = i < countWrites - 1 ? PIPE_BUF : bufferToWrite.size() - offset;
        const ssize_t resultWrite = ::write(fileDescriptor, bufferToWrite.data() + offset, countBytesToSend);
        NET_CHECK(resultWrite, -1L);
        const int resultClose = ::close(fileDescriptor);
        NET_CHECK(resultClose, -1);
        offset += PIPE_BUF;
    }
}

void NamedPipe::read(Buffer& buffer) {
    const int fileDescriptor = ::open(m_pipePath.c_str(), O_RDONLY);
    NET_CHECK(fileDescriptor, -1);
    const ssize_t readBytes = ::read(fileDescriptor, buffer.data.data(), buffer.data.size());
    NET_CHECK(readBytes, -1L);
    buffer.countBytes = readBytes;
    const int resultClose = ::close(fileDescriptor);
    NET_CHECK(resultClose, -1);
}

} // namespace Common