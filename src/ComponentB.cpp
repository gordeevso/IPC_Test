#include <cassert>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "Settings.h"
#include "Common.h"
#include "EntriesProcessing.h"

void readerFromComponentA(std::shared_ptr<Common::ThreadSafeQueueBuffer> threadSafeQueueBufferPtr, std::string_view pipePath) {
    using namespace Common;
    try {
        NamedPipe readerPipe(pipePath);
        while (true) {
            Buffer &buffer = threadSafeQueueBufferPtr->dequeueReadyToUse();
            readerPipe.read(buffer);
            threadSafeQueueBufferPtr->enqueueInProcess(&buffer);
        }
    } catch (std::exception& e) {
        std::cerr << "Error from readerFromComponentA: " << e.what() << std::endl;
    }
}

void tryReconnectWhileRefused(std::shared_ptr<Common::NetworkReaderWriter<Common::ProtocolType::TCP>>& readerWriterTcpPtr, std::uint16_t port, std::string_view ipv4Address, std::chrono::milliseconds retryInterval) {
    using namespace Common;
    while (true) {
        const int resultConnect = readerWriterTcpPtr->reConnect(port, ipv4Address.data());
        if (resultConnect == -1) {
            if (errno != ECONNREFUSED) {
                NET_CHECK(resultConnect, -1);
            }
            // Server is not running yet, try until success
            std::cout << "Trying to reconnect to port: " << port << " host: " << ipv4Address << std::endl;
            std::this_thread::sleep_for(retryInterval);
        } else {
            std::cout << "Successfully connected to port " << port << " host: " << ipv4Address << std::endl;
            break;
        }
    }
}

void readerFromExternalServer(std::shared_ptr<Common::NetworkReaderWriter<Common::ProtocolType::TCP>> readerWriterTcpPtr, std::uint16_t port, std::string_view ipv4Address) {
    using namespace Common;
    try {
        std::vector<std::uint8_t> buffer(PIPE_BUF);
        while(true) {
            std::int64_t readBytes = readerWriterTcpPtr->read(buffer);
            if(readBytes > 0) {
                for (std::size_t index = 0; index < readBytes; ++index) {
                    std::cout << buffer[index];
                }
            } else {
                tryReconnectWhileRefused(readerWriterTcpPtr, port, ipv4Address, std::chrono::milliseconds (Settings::RECONNECT_RETRY_INTERVAL_MILLISECONDS));
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Error from readerFromExternalServer: " << e.what() << std::endl;
    }
}

void writerToExternalServer(std::shared_ptr<Common::ThreadSafeQueueBuffer> threadSafeQueueBufferPtr, std::uint16_t port, std::string_view ipv4Address) {
    using namespace Common;
    std::shared_ptr<NetworkReaderWriter<ProtocolType::TCP>> readerWriterTcpPtr = std::make_shared<NetworkReaderWriter<ProtocolType::TCP>>(port, ipv4Address);

    std::thread readerThread (readerFromExternalServer, readerWriterTcpPtr, port, ipv4Address);
    readerThread.detach();

    std::vector<std::vector<std::uint8_t>> pricesToSend;
    pricesToSend.reserve(PIPE_BUF);
    for(std::vector<std::uint8_t>& buffer : pricesToSend) {
        buffer.reserve(Settings::MESSAGE_TO_EXTERNAL_SERVER_SIZE);
    }

    while(true) {
        Buffer& prices = threadSafeQueueBufferPtr->dequeueInProcess();

        NET_ASSERT(prices.countBytes <= pricesToSend.capacity());
        Processing::filterPrices(prices, pricesToSend, Settings::MESSAGE_TO_EXTERNAL_SERVER_SIZE, [](const std::uint8_t price) {
            return price > Settings::THRESHOLD_PRICE;
        });

        // According to the task we should send single message (eg. 88 88 88 88 88) to external server
        for (std::int32_t index = 0; index < pricesToSend.size(); ++index) {
            const std::int64_t result = readerWriterTcpPtr->write(pricesToSend[index]);
            if (result == -1) {
                tryReconnectWhileRefused(readerWriterTcpPtr, port, ipv4Address, std::chrono::milliseconds (Settings::RECONNECT_RETRY_INTERVAL_MILLISECONDS));
                --index;
            }
        }

        threadSafeQueueBufferPtr->enqueueUsed(&prices);
    }
}

void terminationSignalHandler(int signal) {
    ::unlink(Settings::PIPE_PATH);
    std::exit(signal);
}

int main(int argc, char *argv[]) {
    using namespace Common;
    if(argc != 2 && argc != 3) {
        std::cerr << "Wrong arguments, usage: ./ComponentB [port] [external server address (ipv4), or empty for localhost]" << std::endl;
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

        std::thread readerThread(readerFromComponentA, threadSafeQueueBufferPtr, Settings::PIPE_PATH);
        readerThread.detach();

        writerToExternalServer(threadSafeQueueBufferPtr, port, argc == 3 ? argv[2] : Settings::LOCAL_HOST);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
