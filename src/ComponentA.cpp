#include <cassert>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "Settings.h"
#include "Common.h"
#include "EntriesProcessing.h"

void readerOfEntries(std::shared_ptr<Common::ThreadSafeQueueBuffer> threadSafeQueueBufferPtr, std::uint16_t port) {
    using namespace Common;
    try {
        NetworkReaderWriter<ProtocolType::UDP> readerUdp(port);
        while (true) {
            Buffer &entries = threadSafeQueueBufferPtr->dequeueReadyToUse();
            entries.countBytes = readerUdp.read(entries.data);
            NET_ASSERT(entries.countBytes <= entries.data.size());
            threadSafeQueueBufferPtr->enqueueInProcess(&entries);
        }
    } catch (std::exception& e) {
        std::cerr << "Error from readerOfEntries: " << e.what() << std::endl;
    }
}

void writerToComponentB(std::shared_ptr<Common::ThreadSafeQueueBuffer>& threadSafeQueueBufferPtr, std::string_view pipePath) {
    using namespace Common;
    NamedPipe namedPipe(pipePath);
    std::vector<std::uint8_t> pricesToSend;
    pricesToSend.reserve(Settings::MAX_UDP_BUF);
    while(true) {
        Buffer& entries = threadSafeQueueBufferPtr->dequeueInProcess();

        // Filter entries and remove unnecessary data (volume)
        // obviously sending less data will help with efficiency of system
        // make sure we do not allocate
        NET_ASSERT(entries.countBytes <= pricesToSend.capacity());
        NET_ASSERT(entries.data.size() >= entries.countBytes);
        // Validate input data and filter prices, if not valid skip
        // any deviation from pattern price volume EOF will be skipped
        if(Processing::filterEntries(entries, pricesToSend, Settings::EOF_MARKER)) {
            // Writes entries from udp to pipe between A and B
            NET_ASSERT(!pricesToSend.empty());
            namedPipe.write(pricesToSend);
        }

        threadSafeQueueBufferPtr->enqueueUsed(&entries);
    }
}

void terminationSignalHandler(int signal) {
    ::unlink(Settings::PIPE_PATH);
    std::exit(signal);
}

int main(int argc, char *argv[]) {
    using namespace Common;
    if(argc != 2) {
        std::cerr << "Wrong arguments, usage: ./ComponentA [port] " << std::endl;
        return -1;
    }

    std::signal(SIGINT, terminationSignalHandler);

    try {
        std::int32_t port = std::stoi(argv[1]);
        if(port > std::numeric_limits<std::uint16_t>::max() || port < 0) {
            std::cerr << "Wrong port number" << std::endl;
            return -1;
        }

        std::shared_ptr<ThreadSafeQueueBuffer> threadSafeQueueBufferPtr = std::make_shared<ThreadSafeQueueBuffer>(PIPE_BUF, Settings::THREAD_SAFE_QUEUE_BUFFER_COUNT_BUFFERS);

        std::thread readerThread(readerOfEntries, threadSafeQueueBufferPtr, port);
        readerThread.detach();

        writerToComponentB(threadSafeQueueBufferPtr, Settings::PIPE_PATH);
    } catch (std::exception& e) {
        std::cerr << "Error from writerToComponentB: " << e.what() << std::endl;
    }

    return 0;
}