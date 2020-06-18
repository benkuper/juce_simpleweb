#ifndef SIMPLE_WEB_CRYPTO_HPP
#define SIMPLE_WEB_CRYPTO_HPP

#include <cmath>
#include <iomanip>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#if USE_OPENSSL
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#endif

namespace SimpleWeb {
	// TODO 2017: remove workaround for MSVS 2012
#if _MSC_VER == 1700                       // MSVS 2012 has no definition for round()
	inline double round(double x) noexcept { // Custom definition of round() for positive numbers
		return floor(x + 0.5);
	}
#endif

	class Crypto {
		const static std::size_t buffer_size = 131072;

	public:
		class Base64 {
		public:

			/// Returns Base64 encoded string from input string.
			static std::string encode(const std::string& input) noexcept {
#if USE_OPENSSL
				std::string base64;

				BIO* bio, * b64;
				BUF_MEM* bptr = BUF_MEM_new();

				b64 = BIO_new(BIO_f_base64());
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
				bio = BIO_new(BIO_s_mem());
				BIO_push(b64, bio);
				BIO_set_mem_buf(b64, bptr, BIO_CLOSE);

				// Write directly to base64-buffer to avoid copy
				auto base64_length = static_cast<std::size_t>(round(4 * ceil(static_cast<double>(input.size()) / 3.0)));
				base64.resize(base64_length);
				bptr->length = 0;
				bptr->max = base64_length + 1;
				bptr->data = &base64[0];

				if (BIO_write(b64, &input[0], static_cast<int>(input.size())) <= 0 || BIO_flush(b64) <= 0)
					base64.clear();

				// To keep &base64[0] through BIO_free_all(b64)
				bptr->length = 0;
				bptr->max = 0;
				bptr->data = nullptr;

				BIO_free_all(b64);
				return base64;
#else
				static constexpr char sEncodingTable[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
				};

				size_t in_len = input.size();
				size_t out_len = 4 * ((in_len + 2) / 3);
				std::string ret(out_len, '\0');
				size_t i;
				char* p = const_cast<char*>(ret.c_str());

				for (i = 0; i < in_len - 2; i += 3) {
					*p++ = sEncodingTable[(input[i] >> 2) & 0x3F];
					*p++ = sEncodingTable[((input[i] & 0x3) << 4) | ((int)(input[i + 1] & 0xF0) >> 4)];
					*p++ = sEncodingTable[((input[i + 1] & 0xF) << 2) | ((int)(input[i + 2] & 0xC0) >> 6)];
					*p++ = sEncodingTable[input[i + 2] & 0x3F];
				}
				if (i < in_len) {
					*p++ = sEncodingTable[(input[i] >> 2) & 0x3F];
					if (i == (in_len - 1)) {
						*p++ = sEncodingTable[((input[i] & 0x3) << 4)];
						*p++ = '=';
					}
					else {
						*p++ = sEncodingTable[((input[i] & 0x3) << 4) | ((int)(input[i + 1] & 0xF0) >> 4)];
						*p++ = sEncodingTable[((input[i + 1] & 0xF) << 2)];
					}
					*p++ = '=';
				}

				return ret;
#endif

			}

			/// Returns Base64 decoded string from base64 input.
			static std::string decode(const std::string& base64) noexcept {
#if USE_OPENSSL
				std::string ascii;

				// Resize ascii, however, the size is a up to two bytes too large.
				ascii.resize((6 * base64.size()) / 8);
				BIO* b64, * bio;

				b64 = BIO_new(BIO_f_base64());
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
				// TODO: Remove in 2022 or later
#if(defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x1000214fL) || (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x2080000fL)
				bio = BIO_new_mem_buf(const_cast<char*>(&base64[0]), static_cast<int>(base64.size()));
#else
				bio = BIO_new_mem_buf(&base64[0], static_cast<int>(base64.size()));
#endif
				bio = BIO_push(b64, bio);

				auto decoded_length = BIO_read(bio, &ascii[0], static_cast<int>(ascii.size()));
				if (decoded_length > 0)
					ascii.resize(static_cast<std::size_t>(decoded_length));
				else
					ascii.clear();

				BIO_free_all(b64);

				return ascii;
#else
				static constexpr unsigned char kDecodingTable[] = {
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
				64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
				15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
				64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
				41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
				};

				size_t in_len = base64.size();
				if (in_len % 4 != 0) return "Input data size is not a multiple of 4";

				size_t out_len = in_len / 4 * 3;
				if (base64[in_len - 1] == '=') out_len--;
				if (base64[in_len - 2] == '=') out_len--;

				std::string ascii;
				ascii.resize(out_len);

				for (size_t i = 0, j = 0; i < in_len;) {
					uint32_t a = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
					uint32_t b = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
					uint32_t c = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
					uint32_t d = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];

					uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

					if (j < out_len) ascii[j++] = (triple >> 2 * 8) & 0xFF;
					if (j < out_len) ascii[j++] = (triple >> 1 * 8) & 0xFF;
					if (j < out_len) ascii[j++] = (triple >> 0 * 8) & 0xFF;
				}

				return ascii;
			}

#endif
		};

		/// Returns hex string from bytes in input string.

		static std::string to_hex_string(const std::string& input) noexcept {
			std::stringstream hex_stream;
			hex_stream << std::hex << std::internal << std::setfill('0');
			for (auto& byte : input)
				hex_stream << std::setw(2) << static_cast<int>(static_cast<unsigned char>(byte));
			return hex_stream.str();
		}

		/// Returns md5 hash value from input string.
#if USE_OPENSSL
		static std::string md5(const std::string& input, std::size_t iterations = 1) noexcept {
			std::string hash;

			hash.resize(128 / 8);
			MD5(reinterpret_cast<const unsigned char*>(&input[0]), input.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			for (std::size_t c = 1; c < iterations; ++c)
				MD5(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}

		/// Returns md5 hash value from input stream.
		static std::string md5(std::istream& stream, std::size_t iterations = 1) noexcept {
			MD5_CTX context;
			MD5_Init(&context);
			std::streamsize read_length;
			std::vector<char> buffer(buffer_size);
			while ((read_length = stream.read(&buffer[0], buffer_size).gcount()) > 0)
				MD5_Update(&context, buffer.data(), static_cast<std::size_t>(read_length));
			std::string hash;
			hash.resize(128 / 8);
			MD5_Final(reinterpret_cast<unsigned char*>(&hash[0]), &context);

			for (std::size_t c = 1; c < iterations; ++c)
				MD5(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}
#endif


#if !USE_OPENSSL
		static int is_big_endian(void) {
			static const int n = 1;
			return ((char*)&n)[0] == 0;
		}

		union char64long16 { unsigned char c[64]; uint32_t l[16]; };

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

		static uint32_t blk0(union char64long16* block, int i) {
			// Forrest: SHA expect BIG_ENDIAN, swap if LITTLE_ENDIAN
			if (!is_big_endian()) {
				block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) |
					(rol(block->l[i], 8) & 0x00FF00FF);
			}
			return block->l[i];
		}

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(block, i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

		struct SHA1_CTX {
			uint32_t state[5];
			uint32_t count[2];
			unsigned char buffer[64];
		};

		static void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]) {
			uint32_t a, b, c, d, e;
			union char64long16 block[1];

			memcpy(block, buffer, 64);
			a = state[0];
			b = state[1];
			c = state[2];
			d = state[3];
			e = state[4];
			R0(a, b, c, d, e, 0); R0(e, a, b, c, d, 1); R0(d, e, a, b, c, 2); R0(c, d, e, a, b, 3);
			R0(b, c, d, e, a, 4); R0(a, b, c, d, e, 5); R0(e, a, b, c, d, 6); R0(d, e, a, b, c, 7);
			R0(c, d, e, a, b, 8); R0(b, c, d, e, a, 9); R0(a, b, c, d, e, 10); R0(e, a, b, c, d, 11);
			R0(d, e, a, b, c, 12); R0(c, d, e, a, b, 13); R0(b, c, d, e, a, 14); R0(a, b, c, d, e, 15);
			R1(e, a, b, c, d, 16); R1(d, e, a, b, c, 17); R1(c, d, e, a, b, 18); R1(b, c, d, e, a, 19);
			R2(a, b, c, d, e, 20); R2(e, a, b, c, d, 21); R2(d, e, a, b, c, 22); R2(c, d, e, a, b, 23);
			R2(b, c, d, e, a, 24); R2(a, b, c, d, e, 25); R2(e, a, b, c, d, 26); R2(d, e, a, b, c, 27);
			R2(c, d, e, a, b, 28); R2(b, c, d, e, a, 29); R2(a, b, c, d, e, 30); R2(e, a, b, c, d, 31);
			R2(d, e, a, b, c, 32); R2(c, d, e, a, b, 33); R2(b, c, d, e, a, 34); R2(a, b, c, d, e, 35);
			R2(e, a, b, c, d, 36); R2(d, e, a, b, c, 37); R2(c, d, e, a, b, 38); R2(b, c, d, e, a, 39);
			R3(a, b, c, d, e, 40); R3(e, a, b, c, d, 41); R3(d, e, a, b, c, 42); R3(c, d, e, a, b, 43);
			R3(b, c, d, e, a, 44); R3(a, b, c, d, e, 45); R3(e, a, b, c, d, 46); R3(d, e, a, b, c, 47);
			R3(c, d, e, a, b, 48); R3(b, c, d, e, a, 49); R3(a, b, c, d, e, 50); R3(e, a, b, c, d, 51);
			R3(d, e, a, b, c, 52); R3(c, d, e, a, b, 53); R3(b, c, d, e, a, 54); R3(a, b, c, d, e, 55);
			R3(e, a, b, c, d, 56); R3(d, e, a, b, c, 57); R3(c, d, e, a, b, 58); R3(b, c, d, e, a, 59);
			R4(a, b, c, d, e, 60); R4(e, a, b, c, d, 61); R4(d, e, a, b, c, 62); R4(c, d, e, a, b, 63);
			R4(b, c, d, e, a, 64); R4(a, b, c, d, e, 65); R4(e, a, b, c, d, 66); R4(d, e, a, b, c, 67);
			R4(c, d, e, a, b, 68); R4(b, c, d, e, a, 69); R4(a, b, c, d, e, 70); R4(e, a, b, c, d, 71);
			R4(d, e, a, b, c, 72); R4(c, d, e, a, b, 73); R4(b, c, d, e, a, 74); R4(a, b, c, d, e, 75);
			R4(e, a, b, c, d, 76); R4(d, e, a, b, c, 77); R4(c, d, e, a, b, 78); R4(b, c, d, e, a, 79);
			state[0] += a;
			state[1] += b;
			state[2] += c;
			state[3] += d;
			state[4] += e;
			// Erase working structures. The order of operations is important,
			// used to ensure that compiler doesn't optimize those out.
			memset(block, 0, sizeof(block));
			a = b = c = d = e = block[0].l[0];
		}

		static void SHA1Init(SHA1_CTX* context) {
			context->state[0] = 0x67452301;
			context->state[1] = 0xEFCDAB89;
			context->state[2] = 0x98BADCFE;
			context->state[3] = 0x10325476;
			context->state[4] = 0xC3D2E1F0;
			context->count[0] = context->count[1] = 0;
		}

		static void SHA1Update(SHA1_CTX* context, const unsigned char* data,
			uint32_t len) {
			uint32_t i, j;

			j = context->count[0];
			if ((context->count[0] += len << 3) < j)
				context->count[1]++;
			context->count[1] += (len >> 29);
			j = (j >> 3) & 63;
			if ((j + len) > 63) {
				memcpy(&context->buffer[j], data, (i = 64 - j));
				SHA1Transform(context->state, context->buffer);
				for (; i + 63 < len; i += 64) {
					SHA1Transform(context->state, &data[i]);
				}
				j = 0;
			}
			else i = 0;
			memcpy(&context->buffer[j], &data[i], len - i);
		}

		static void SHA1Final(unsigned char digest[20], SHA1_CTX* context) {
			unsigned i;
			unsigned char finalcount[8], c;

			for (i = 0; i < 8; i++) {
				finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
					>> ((3 - (i & 3)) * 8)) & 255);
			}
			c = 0200;
			SHA1Update(context, &c, 1);
			while ((context->count[0] & 504) != 448) {
				c = 0000;
				SHA1Update(context, &c, 1);
			}
			SHA1Update(context, finalcount, 8);
			for (i = 0; i < 20; i++) {
				digest[i] = (unsigned char)
					((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
			}
			memset(context, '\0', sizeof(*context));
			memset(&finalcount, '\0', sizeof(finalcount));
		}
#endif




		/// Returns sha1 hash value from input string.
		static std::string sha1(const std::string& input, std::size_t /*iterations */ = 1) noexcept {
#if USE_OPENSSL
			std::string hash;

			hash.resize(160 / 8);
			SHA1(reinterpret_cast<const unsigned char*>(&input[0]), input.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			for (std::size_t c = 1; c < iterations; ++c)
				SHA1(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
#else
			char sha[20];
			SHA1_CTX sha_ctx;
			SHA1Init(&sha_ctx);
			SHA1Update(&sha_ctx, (const unsigned char*)input.c_str(), (uint32_t)input.size());
			SHA1Final((unsigned char*)sha, &sha_ctx);

			return std::string(sha, sizeof(sha));
#endif
		}

		/// Returns sha1 hash value from input stream.
		static std::string sha1(std::istream& stream, std::size_t iterations = 1) noexcept {
#if USE_OPENSSL
			SHA_CTX context;
			SHA1_Init(&context);
			std::streamsize read_length;
			std::vector<char> buffer(buffer_size);
			while ((read_length = stream.read(&buffer[0], buffer_size).gcount()) > 0)
				SHA1_Update(&context, buffer.data(), static_cast<std::size_t>(read_length));
			std::string hash;
			hash.resize(160 / 8);
			SHA1_Final(reinterpret_cast<unsigned char*>(&hash[0]), &context);

			for (std::size_t c = 1; c < iterations; ++c)
				SHA1(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
#else
			SHA1_CTX context;
			SHA1Init(&context);
			std::streamsize read_length;
			std::vector<char> buffer(buffer_size);
			while ((read_length = stream.read(&buffer[0], buffer_size).gcount()) > 0)
				SHA1Update(&context, (const unsigned char*)buffer.data(), static_cast<std::size_t>(read_length));
			
			std::string hash;
			hash.resize(160 / 8);
			SHA1Final(reinterpret_cast<unsigned char*>(&hash[0]), &context);
			return hash;
#endif
		}

#if USE_OPENSSL
		/// Returns sha256 hash value from input string.
		static std::string sha256(const std::string& input, std::size_t iterations = 1) noexcept {
			std::string hash;

			hash.resize(256 / 8);
			SHA256(reinterpret_cast<const unsigned char*>(&input[0]), input.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			for (std::size_t c = 1; c < iterations; ++c)
				SHA256(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}

		/// Returns sha256 hash value from input stream.
		static std::string sha256(std::istream& stream, std::size_t iterations = 1) noexcept {
			SHA256_CTX context;
			SHA256_Init(&context);
			std::streamsize read_length;
			std::vector<char> buffer(buffer_size);
			while ((read_length = stream.read(&buffer[0], buffer_size).gcount()) > 0)
				SHA256_Update(&context, buffer.data(), static_cast<std::size_t>(read_length));
			std::string hash;
			hash.resize(256 / 8);
			SHA256_Final(reinterpret_cast<unsigned char*>(&hash[0]), &context);

			for (std::size_t c = 1; c < iterations; ++c)
				SHA256(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}

		/// Returns sha512 hash value from input string.
		static std::string sha512(const std::string& input, std::size_t iterations = 1) noexcept {
			std::string hash;

			hash.resize(512 / 8);
			SHA512(reinterpret_cast<const unsigned char*>(&input[0]), input.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			for (std::size_t c = 1; c < iterations; ++c)
				SHA512(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}

		/// Returns sha512 hash value from input stream.
		static std::string sha512(std::istream& stream, std::size_t iterations = 1) noexcept {
			SHA512_CTX context;
			SHA512_Init(&context);
			std::streamsize read_length;
			std::vector<char> buffer(buffer_size);
			while ((read_length = stream.read(&buffer[0], buffer_size).gcount()) > 0)
				SHA512_Update(&context, buffer.data(), static_cast<std::size_t>(read_length));
			std::string hash;
			hash.resize(512 / 8);
			SHA512_Final(reinterpret_cast<unsigned char*>(&hash[0]), &context);

			for (std::size_t c = 1; c < iterations; ++c)
				SHA512(reinterpret_cast<const unsigned char*>(&hash[0]), hash.size(), reinterpret_cast<unsigned char*>(&hash[0]));

			return hash;
		}

		/// Returns PBKDF2 hash value from the given password
		/// Input parameter key_size  number of bytes of the returned key.

		/**
		 * Returns PBKDF2 derived key from the given password.
		 *
		 * @param password   The password to derive key from.
		 * @param salt       The salt to be used in the algorithm.
		 * @param iterations Number of iterations to be used in the algorithm.
		 * @param key_size   Number of bytes of the returned key.
		 *
		 * @return The PBKDF2 derived key.
		 */
		static std::string pbkdf2(const std::string& password, const std::string& salt, int iterations, int key_size) noexcept {
			std::string key;
			key.resize(static_cast<std::size_t>(key_size));
			PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.size(),
				reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size(), iterations,
				key_size, reinterpret_cast<unsigned char*>(&key[0]));
			return key;
		}
#endif

	};
} // namespace SimpleWeb
#endif /* SIMPLE_WEB_CRYPTO_HPP */
