#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstring>
#include "tcp_recv_buffer.h"

using namespace std;

TEST(tcp_recv_buffer_test, normal_read_write) {
    crpc::RecvBuffer recvBuffer(20);

    vector<char> vec(10, 0);
    char* data = vec.data();
    memcpy(data, "123456789", 10);
    int hasWriteByte = recvBuffer.AppendBuffer(vec, 0, vec.size());

    vector<char> out(10, 0);
    recvBuffer.GetBuffer(10, out);
    EXPECT_EQ(hasWriteByte, 10);
    EXPECT_EQ(*vec.data(), *out.data());
}

TEST(tcp_recv_buffer_test, loop_read_write) {
    crpc::RecvBuffer recvBuffer(10);
    vector<char> vec(7, 0);
    char* data = vec.data();
    memcpy(data, "1234567", 7);
    recvBuffer.AppendBuffer(vec, 0, vec.size());
    vector<char> out(3, 0);
    recvBuffer.GetBuffer(3, out); // 缓冲区剩下4567

    vector<char> vec1(6, 0);
    char* data1 = vec1.data();
    memcpy(data1, "abcdef", 6);
    recvBuffer.AppendBuffer(vec1, 0, vec1.size());

    vector<char> out1(10, 0);
    recvBuffer.GetBuffer(10, out1);
    string expect = "4567abcdef";
    EXPECT_EQ(*expect.c_str(), *out1.data());
}

TEST(tcp_recv_buffer_test, insufficient_space_write) {
    crpc::RecvBuffer recvBuffer(5);

    vector<char> vec(10, 0);
    char* data = vec.data();
    memcpy(data, "123456789", 10);
    int hasWriteByte = recvBuffer.AppendBuffer(vec, 0, vec.size());

    vector<char> out(5, 0);
    recvBuffer.GetBuffer(5, out);
    EXPECT_EQ(hasWriteByte, 5);
    EXPECT_EQ(*vec.data(), *out.data());
}

TEST(tcp_recv_buffer_test, insufficient_space_write2) {
    crpc::RecvBuffer recvBuffer(10);

    vector<char> vec(6, 0);
    char* data = vec.data();
    memcpy(data, "123456", 6);
    recvBuffer.AppendBuffer(vec, 0, vec.size());

    vector<char> vec1(5, 0);
    char* data1 = vec1.data();
    memcpy(data1, "abcde", 5);
    int hasWriteByte = recvBuffer.AppendBuffer(vec1, 0, vec1.size()); // 先写一部分，再写超出缓冲区大小

    vector<char> out(10, 0);
    recvBuffer.GetBuffer(10, out);
    EXPECT_EQ(hasWriteByte, 4);
    string expect = "1234566abcd";
    EXPECT_EQ(*expect.c_str(), *out.data());
}

TEST(tcp_recv_buffer_test, loop_insufficient_space_write) {
    crpc::RecvBuffer recvBuffer(10);
    vector<char> vec(7, 0);
    char* data = vec.data();
    memcpy(data, "1234567", 7);
    recvBuffer.AppendBuffer(vec, 0, vec.size());
    vector<char> out(3, 0);
    recvBuffer.GetBuffer(3, out); // 缓冲区剩下4567

    vector<char> vec1(7, 0);
    char* data1 = vec1.data();
    memcpy(data1, "abcdefg", 7);
    int hasWriteByte = recvBuffer.AppendBuffer(vec1, 0, vec1.size());

    vector<char> out1(10, 0);
    recvBuffer.GetBuffer(10, out1);
    string expect = "4567abcdef";
    EXPECT_EQ(*expect.c_str(), *out1.data());
    EXPECT_EQ(hasWriteByte, 6);
}

TEST(tcp_recv_buffer_test, insufficient_byte_for_read) {
    crpc::RecvBuffer recvBuffer(10);

    vector<char> vec(3, 0);
    char* data = vec.data();
    memcpy(data, "123", 3);
    recvBuffer.AppendBuffer(vec, 0, vec.size());
    vector<char> out(5, 0);
    bool ret = recvBuffer.GetBuffer(5, out);

    EXPECT_EQ(ret, false);

    recvBuffer.GetBuffer(3, out);
    string expect = "123";
    EXPECT_EQ(*expect.c_str(), *out.data());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
