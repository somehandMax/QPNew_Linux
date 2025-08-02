#ifndef HASHCRYPTO_H
#define HASHCRYPTO_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdint> 

class SecureHash {
private:
	static constexpr uint32_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	static inline uint32_t rotr(uint32_t x, uint32_t n) {
		return (x >> n) | (x << (32 - n));
	}
	static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (~x & z);
	}
	
	static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (x & z) ^ (y & z);
	}
	
	static inline uint32_t sigma0(uint32_t x) {
		return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
	}
	
	static inline uint32_t sigma1(uint32_t x) {
		return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
	}
	
	static inline uint32_t gamma0(uint32_t x) {
		return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
	}
	
	static inline uint32_t gamma1(uint32_t x) {
		return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
	}
	
	// 初始化哈希值
	static void initHash(uint32_t hash[8]) {
		hash[0] = 0x6a09e667;
		hash[1] = 0xbb67ae85;
		hash[2] = 0x3c6ef372;
		hash[3] = 0xa54ff53a;
		hash[4] = 0x510e527f;
		hash[5] = 0x9b05688c;
		hash[6] = 0x1f83d9ab;
		hash[7] = 0x5be0cd19;
	}
	static void processBlock(const uint8_t* block, uint32_t hash[8]) {
		uint32_t w[64];
		for (int i = 0; i < 16; i++) {
			w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
			(static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
			(static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
			(static_cast<uint32_t>(block[i * 4 + 3]));
		}
		
		for (int i = 16; i < 64; i++) {
			w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
		}
		uint32_t a = hash[0];
		uint32_t b = hash[1];
		uint32_t c = hash[2];
		uint32_t d = hash[3];
		uint32_t e = hash[4];
		uint32_t f = hash[5];
		uint32_t g = hash[6];
		uint32_t h = hash[7];
		for (int i = 0; i < 64; i++) {
			uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
			uint32_t t2 = sigma0(a) + maj(a, b, c);
			
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
		hash[0] += a;
		hash[1] += b;
		hash[2] += c;
		hash[3] += d;
		hash[4] += e;
		hash[5] += f;
		hash[6] += g;
		hash[7] += h;
	}
	
public:
	// 计算SHA-256哈希值
	static std::string sha256(const std::string& input) {
		// 初始化哈希值
		uint32_t hash[8];
		initHash(hash);
		
		// 处理输入数据
		size_t input_len = input.length();
		size_t total_len = input_len + 1 + 8; // 数据 + 1字节0x80 + 8字节长度
		size_t padding_len = (56 - (total_len % 64)) % 64;
		size_t final_len = input_len + 1 + padding_len + 8;
		
		// 创建填充数据
		std::vector<uint8_t> data(final_len);
		std::memcpy(data.data(), input.c_str(), input_len);
		data[input_len] = 0x80;
		for (size_t i = input_len + 1; i < input_len + 1 + padding_len; i++) {
			data[i] = 0x00;
		}
		uint64_t bit_len = static_cast<uint64_t>(input_len) * 8;
		for (int i = 0; i < 8; i++) {
			data[final_len - 8 + i] = (bit_len >> (56 - 8 * i)) & 0xff;
		}
		for (size_t i = 0; i < final_len; i += 64) {
			processBlock(data.data() + i, hash);
		}
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		for (int i = 0; i < 8; i++) {
			ss << std::setw(8) << hash[i];
		}
		return ss.str();
	}
	static std::string passwordHash(const std::string& password, const std::string& salt = "") {
		std::string salted_input = password + salt;
		return sha256(salted_input);
	}
	static bool passwordComparison(const std::string& passwordInput, const std::string& passwordCheck) {
		if ( (passwordCheck.compare(passwordInput)) == 0 )return true;
		else return false;
	}
};

#endif
//SHA-256哈希	SecureHash::sha256(password)
//带盐哈希	SecureHash::passwordHash(password, salt)
//哈希值比对 SecureHash::passwordComparison(待检查password,校验password)