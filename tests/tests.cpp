#include <gtest/gtest.h>

#include "Common.h"
#include "EntriesProcessing.h"

using namespace Common;
using namespace Processing;

// filterEntries

TEST(ProcessingTests, FilterEntries_1) {
    Buffer entries;
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_2) {
    Buffer entries;
    entries.data = {1};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_3) {
    Buffer entries;
    entries.data = {1, 2};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_4) {
    Buffer entries;
    entries.data = {1, 2};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_5) {
    Buffer entries;
    entries.data = {1, 2, '\n'};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_TRUE(isGood);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 1);
}

TEST(ProcessingTests, FilterEntries_6) {
    Buffer entries;
    entries.data = {1, 2, '\n', 3};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_TRUE(isGood);
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0], 1);
}

TEST(ProcessingTests, FilterEntries_7) {
    Buffer entries;
    entries.data = {1, 2, 3, '\n'};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_8) {
    Buffer entries;
    entries.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, '\n', 11, 12, 13};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_TRUE(isGood);
    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 3);
    ASSERT_EQ(result[2], 5);
    ASSERT_EQ(result[3], 7);
    ASSERT_EQ(result[4], 9);
}

TEST(ProcessingTests, FilterEntries_9) {
    Buffer entries;
    entries.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, '\n', 11, 12, 13};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_FALSE(isGood);
}

TEST(ProcessingTests, FilterEntries_10) {
    Buffer entries;
    entries.data = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, '\n'};
    entries.countBytes = entries.data.size();
    std::vector<std::uint8_t> result;
    bool isGood = filterEntries(entries, result, '\n');
    ASSERT_TRUE(isGood);
    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 1);
    ASSERT_EQ(result[2], 1);
    ASSERT_EQ(result[3], 1);
    ASSERT_EQ(result[4], 1);
}

// filterPrices

TEST(ProcessingTests, FilterPrices_1) {
    Buffer allPrices;
    std::vector<std::vector<std::uint8_t>> result;
    filterPrices(allPrices, result, 5, [](std::uint8_t price){
        return price > 80;
    });
    ASSERT_TRUE(result.empty());
}

TEST(ProcessingTests, FilterPrices_2) {
    Buffer allPrices;
    allPrices.data = {10};
    allPrices.countBytes = allPrices.data.size();
    std::vector<std::vector<std::uint8_t>> result;
    filterPrices(allPrices, result, 5, [](std::uint8_t price){
        return price > 80;
    });
    ASSERT_TRUE(result.empty());
}

TEST(ProcessingTests, FilterPrices_3) {
    Buffer allPrices;
    allPrices.data = {80};
    allPrices.countBytes = allPrices.data.size();
    std::vector<std::vector<std::uint8_t>> result;
    filterPrices(allPrices, result, 5, [](std::uint8_t price){
        return price > 80;
    });
    ASSERT_TRUE(result.empty());
}

TEST(ProcessingTests, FilterPrices_4) {
    Buffer allPrices;
    allPrices.data = {80, 90, 30};
    allPrices.countBytes = allPrices.data.size();
    std::vector<std::vector<std::uint8_t>> result;
    filterPrices(allPrices, result, 5, [](std::uint8_t price){
        return price > 80;
    });
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].size(), 5);
    ASSERT_EQ(result[0][0], 90);
    ASSERT_EQ(result[0][1], 90);
    ASSERT_EQ(result[0][2], 90);
    ASSERT_EQ(result[0][3], 90);
    ASSERT_EQ(result[0][4], 90);
}

TEST(ProcessingTests, FilterPrices_5) {
    Buffer allPrices;
    allPrices.data = {80, 90, 30, 91, 100};
    allPrices.countBytes = allPrices.data.size();
    std::vector<std::vector<std::uint8_t>> result;
    filterPrices(allPrices, result, 5, [](std::uint8_t price){
        return price > 80;
    });
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0].size(), 5);
    ASSERT_EQ(result[0][0], 90);
    ASSERT_EQ(result[0][1], 90);
    ASSERT_EQ(result[0][2], 90);
    ASSERT_EQ(result[0][3], 90);
    ASSERT_EQ(result[0][4], 90);
    ASSERT_EQ(result[1].size(), 5);
    ASSERT_EQ(result[1][0], 91);
    ASSERT_EQ(result[1][1], 91);
    ASSERT_EQ(result[1][2], 91);
    ASSERT_EQ(result[1][3], 91);
    ASSERT_EQ(result[1][4], 91);
    ASSERT_EQ(result[2].size(), 5);
    ASSERT_EQ(result[2][0], 100);
    ASSERT_EQ(result[2][1], 100);
    ASSERT_EQ(result[2][2], 100);
    ASSERT_EQ(result[2][3], 100);
    ASSERT_EQ(result[2][4], 100);
}

// ThreadSafeQueueBuffer

TEST(CommonTests, ThreadSafeQueueBuffer_1) {
    ThreadSafeQueueBuffer tsBuffer(0, 0);
    ASSERT_EQ(tsBuffer.countToUse(), 0);
    ASSERT_EQ(tsBuffer.capacityToUse(), 0);
    ASSERT_EQ(tsBuffer.countInProcess(), 0);
    ASSERT_EQ(tsBuffer.capacityInProcess(), 0);
}

TEST(CommonTests, ThreadSafeQueueBuffer_2) {
    ThreadSafeQueueBuffer tsBuffer(0, 1);
    ASSERT_EQ(tsBuffer.countToUse(), 1);
    ASSERT_EQ(tsBuffer.countInProcess(), 0);
    for(int i = 0; i < 100; ++i) {
        Buffer &bufferToFill = tsBuffer.dequeueReadyToUse();
        ASSERT_EQ(tsBuffer.countToUse(), 0);
        ASSERT_EQ(tsBuffer.countInProcess(), 0);
        tsBuffer.enqueueInProcess(&bufferToFill);
        ASSERT_EQ(tsBuffer.countToUse(), 0);
        ASSERT_EQ(tsBuffer.countInProcess(), 1);
        Buffer &bufferFilled = tsBuffer.dequeueInProcess();
        ASSERT_EQ(tsBuffer.countToUse(), 0);
        ASSERT_EQ(tsBuffer.countInProcess(), 0);
        tsBuffer.enqueueUsed(&bufferFilled);
        ASSERT_EQ(tsBuffer.countToUse(), 1);
        ASSERT_EQ(tsBuffer.countInProcess(), 0);
    }
}

TEST(CommonTests, ThreadSafeQueueBuffer_3) {
    ThreadSafeQueueBuffer tsBuffer(0, 1);
    for(int i = 0; i < 100; ++i) {
        Buffer &bufferToFill = tsBuffer.dequeueReadyToUse();
        bufferToFill.data = {1, 2, 3, 4, 5};
        bufferToFill.countBytes = bufferToFill.data.size();
        tsBuffer.enqueueInProcess(&bufferToFill);
        Buffer &bufferFilled = tsBuffer.dequeueInProcess();
        ASSERT_EQ(bufferFilled.countBytes, 5);
        ASSERT_TRUE(bufferFilled.data.size() >= bufferFilled.countBytes);
        ASSERT_EQ(bufferFilled.data[0], 1);
        ASSERT_EQ(bufferFilled.data[1], 2);
        ASSERT_EQ(bufferFilled.data[2], 3);
        ASSERT_EQ(bufferFilled.data[3], 4);
        ASSERT_EQ(bufferFilled.data[4], 5);
        tsBuffer.enqueueUsed(&bufferFilled);
    }
}

TEST(CommonTests, ThreadSafeQueueBuffer_4) {
    ThreadSafeQueueBuffer tsBuffer(16, 3);

    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    std::thread producer([&tsBuffer, &flag](){
        while(!flag.test()){
            std::this_thread::yield();
        }
        for(int i = 0; i < 1000; ++i) {
            Buffer &bufferToFill = tsBuffer.dequeueReadyToUse();
            tsBuffer.enqueueInProcess(&bufferToFill);
        }
    });

    flag.test_and_set();
    for(int i = 0; i < 1000; ++i) {
        Buffer &bufferFilled = tsBuffer.dequeueInProcess();
        tsBuffer.enqueueUsed(&bufferFilled);
    }

    producer.join();

    ASSERT_EQ(tsBuffer.countToUse(), 3);
    ASSERT_EQ(tsBuffer.capacityToUse(), 3);
    ASSERT_EQ(tsBuffer.countInProcess(), 0);
    ASSERT_EQ(tsBuffer.capacityInProcess(), 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}