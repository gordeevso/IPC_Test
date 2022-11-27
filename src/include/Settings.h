#pragma once

namespace Settings {

static constexpr std::uint8_t EOF_MARKER = '\n';
static constexpr std::uint8_t THRESHOLD_PRICE = 80;
static constexpr std::size_t MESSAGE_TO_EXTERNAL_SERVER_SIZE = 5;
static constexpr std::size_t THREAD_SAFE_QUEUE_BUFFER_COUNT_BUFFERS = 32;
static constexpr std::size_t RECONNECT_RETRY_INTERVAL_MILLISECONDS = 1000;
static constexpr std::int32_t MAX_UDP_BUF = 65507;
static constexpr char LOCAL_HOST[] = "localhost";
static constexpr char PIPE_PATH[] = "./fifoAB";

}