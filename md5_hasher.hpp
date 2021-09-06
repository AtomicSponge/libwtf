/*
 * MD5 Hasher
 * By:  Matthew Evans
 * File:  md5_hasher.hpp
 * Version:  082521
 *
 * See LICENSE.md for copyright information.
 * 
 * Calculates the MD5 hash value.
 * https://tools.ietf.org/html/rfc1321
 *
 * Example usage:
 * 
 * std::string get_file_hash(const std::string& file_name) {
 *     FILE* hash_file = fopen(file_name.c_str(), "rb");
 *     if(hash_file == NULL) return "ERROR_FILE_NOT_FOUND";
 * 
 *     md5_hasher my_hasher;
 *     my_hasher.initialize();
 * 
 *     std::size_t len;
 *     unsigned char buffer[1024];
 *     while((len = fread(buffer, 1, 1024, hash_file)) != 0)
 *         my_hasher.update(buffer, len);
 *     my_hasher.finalize();
 *     fclose(hash_file);
 * 
 *     return my_hasher.get_hash();
 * }
 * 
 */

#ifndef WTF_MD5_HASHER_HPP
#define WTF_MD5_HASHER_HPP

#include <string>
#include <sstream>
#include <iomanip>

namespace wtf {

typedef std::uint32_t md5_block;

//  MD5 bitwise functions
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define left_rotate(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

//  Transformations
#define FF(a, b, c, d, x, s, ac) \
  { (a) += F ((b), (c), (d)) + (x) + (md5_block)(ac); \
    (a) = left_rotate ((a), (s)); \
    (a) += (b); }
#define GG(a, b, c, d, x, s, ac) \
  { (a) += G ((b), (c), (d)) + (x) + (md5_block)(ac); \
    (a) = left_rotate ((a), (s)); \
    (a) += (b); }
#define HH(a, b, c, d, x, s, ac) \
  { (a) += H ((b), (c), (d)) + (x) + (md5_block)(ac); \
    (a) = left_rotate ((a), (s)); \
    (a) += (b); }
#define II(a, b, c, d, x, s, ac) \
  { (a) += I ((b), (c), (d)) + (x) + (md5_block)(ac); \
    (a) = left_rotate ((a), (s)); \
    (a) += (b); }

/*!
 * \class md5_hasher
 * \brief Calculate md5 hash value.
 */
class md5_hasher {
    public:
        /*!
         * \brief Return the calculated hash as a string
         * \return MD5 hash value
         */
        inline const std::string get_hash(void) {
            std::ostringstream oss;

            for (std::size_t i = 0; i < 16; i++) {
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
            }

            return oss.str();
        };

        /*
         * Call this first to set up the states
         */
        inline void initialize(void) {
            counter[0] = counter[1] = (md5_block)0;

            buffer[0] = (md5_block)0x67452301;
            buffer[1] = (md5_block)0xefcdab89;
            buffer[2] = (md5_block)0x98badcfe;
            buffer[3] = (md5_block)0x10325476;
        };

        /*
         * Take a block of data and process its hash
         */
        inline void update(const unsigned char* hash_me, std::size_t len) {
            md5_block input[16];
            std::size_t mdi;

            //  Get number of bytes
            mdi = (std::size_t)((counter[0] >> 3) & 0x3F);

            //  Add to the length
            if((counter[0] + ((md5_block)len << 3)) < counter[0])
                counter[1]++;
            counter[0] += ((md5_block)len << 3);
            counter[1] += ((md5_block)len >> 29);

            while(len--) {
                //  Read data into input buffer
                input_buffer[mdi++] = *hash_me++;

                //  Perform transformation
                if(mdi == 0x40) {
                    for(std::size_t i = 0, ii = 0; i < 16; i++, ii += 4) {
                        input[i] = (((md5_block)input_buffer[ii + 3]) << 24) |
                                   (((md5_block)input_buffer[ii + 2]) << 16) |
                                   (((md5_block)input_buffer[ii + 1]) << 8) |
                                    ((md5_block)input_buffer[ii]);
                    }
                    transform(input);
                    mdi = 0;
                }
            }
        };

        /*
         * Finalize MD5 hash and write the digest
         */
        inline void finalize(void) {
            md5_block input[16];
            std::size_t mdi;
            unsigned int padlen;

            //  Assign the size of the input data to the last two blocks
            input[14] = counter[0];
            input[15] = counter[1];

            //  Get number of bytes
            mdi = (std::size_t)((counter[0] >> 3) & 0x3F);

            //  Add padding
            padlen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
            update(PADDING, padlen);

            //  Transform the remaining data
            for(std::size_t i = 0, ii = 0; i < 14; i++, ii += 4) {
                input[i] = (((md5_block)input_buffer[ii + 3]) << 24) |
                           (((md5_block)input_buffer[ii + 2]) << 16) |
                           (((md5_block)input_buffer[ii + 1]) << 8) |
                            ((md5_block)input_buffer[ii]);
            }
            transform(input);

            //  Store the calculated result in the digest
            for (std::size_t i = 0, ii = 0; i < 4; i++, ii += 4) {
                digest[ii] = (unsigned char)(buffer[i] & 0xFF);
                digest[ii + 1] = (unsigned char)((buffer[i] >> 8) & 0xFF);
                digest[ii + 2] = (unsigned char)((buffer[i] >> 16) & 0xFF);
                digest[ii + 3] = (unsigned char)((buffer[i] >> 24) & 0xFF);
            }
        };

    private:
        /*
         * Perform transformations
         */
        inline void transform(const md5_block M[16]) {
            md5_block A = buffer[0];
            md5_block B = buffer[1];
            md5_block C = buffer[2];
            md5_block D = buffer[3];

            //  Round 1
            FF(A, B, C, D, M[ 0],  7, 3614090360);  FF(D, A, B, C, M[ 1], 12, 3905402710);  FF(C, D, A, B, M[ 2], 17, 606105819);   FF(B, C, D, A, M[ 3], 22, 3250441966);
            FF(A, B, C, D, M[ 4],  7, 4118548399);  FF(D, A, B, C, M[ 5], 12, 1200080426);  FF(C, D, A, B, M[ 6], 17, 2821735955);  FF(B, C, D, A, M[ 7], 22, 4249261313);
            FF(A, B, C, D, M[ 8],  7, 1770035416);  FF(D, A, B, C, M[ 9], 12, 2336552879);  FF(C, D, A, B, M[10], 17, 4294925233);  FF(B, C, D, A, M[11], 22, 2304563134);
            FF(A, B, C, D, M[12],  7, 1804603682);  FF(D, A, B, C, M[13], 12, 4254626195);  FF(C, D, A, B, M[14], 17, 2792965006);  FF(B, C, D, A, M[15], 22, 1236535329);

            //  Round 2
            GG(A, B, C, D, M[ 1],  5, 4129170786);  GG(D, A, B, C, M[ 6],  9, 3225465664);  GG(C, D, A, B, M[11], 14, 643717713);   GG(B, C, D, A, M[ 0], 20, 3921069994);
            GG(A, B, C, D, M[ 5],  5, 3593408605);  GG(D, A, B, C, M[10],  9, 38016083);    GG(C, D, A, B, M[15], 14, 3634488961);  GG(B, C, D, A, M[ 4], 20, 3889429448);
            GG(A, B, C, D, M[ 9],  5, 568446438);   GG(D, A, B, C, M[14],  9, 3275163606);  GG(C, D, A, B, M[ 3], 14, 4107603335);  GG(B, C, D, A, M[ 8], 20, 1163531501);
            GG(A, B, C, D, M[13],  5, 2850285829);  GG(D, A, B, C, M[ 2],  9, 4243563512);  GG(C, D, A, B, M[ 7], 14, 1735328473);  GG(B, C, D, A, M[12], 20, 2368359562);

            //  Round 3
            HH(A, B, C, D, M[ 5],  4, 4294588738);  HH(D, A, B, C, M[ 8], 11, 2272392833);  HH(C, D, A, B, M[11], 16, 1839030562);  HH(B, C, D, A, M[14], 23, 4259657740);
            HH(A, B, C, D, M[ 1],  4, 2763975236);  HH(D, A, B, C, M[ 4], 11, 1272893353);  HH(C, D, A, B, M[ 7], 16, 4139469664);  HH(B, C, D, A, M[10], 23, 3200236656);
            HH(A, B, C, D, M[13],  4, 681279174);   HH(D, A, B, C, M[ 0], 11, 3936430074);  HH(C, D, A, B, M[ 3], 16, 3572445317);  HH(B, C, D, A, M[ 6], 23, 76029189);
            HH(A, B, C, D, M[ 9],  4, 3654602809);  HH(D, A, B, C, M[12], 11, 3873151461);  HH(C, D, A, B, M[15], 16, 530742520);   HH(B, C, D, A, M[ 2], 23, 3299628645);

            //  Round 4
            II(A, B, C, D, M[ 0],  6, 4096336452);  II(D, A, B, C, M[ 7], 10, 1126891415);  II(C, D, A, B, M[14], 15, 2878612391);  II(B, C, D, A, M[ 5], 21, 4237533241);
            II(A, B, C, D, M[12],  6, 1700485571);  II(D, A, B, C, M[ 3], 10, 2399980690);  II(C, D, A, B, M[10], 15, 4293915773);  II(B, C, D, A, M[ 1], 21, 2240044497);
            II(A, B, C, D, M[ 8],  6, 1873313359);  II(D, A, B, C, M[15], 10, 4264355552);  II(C, D, A, B, M[ 6], 15, 2734768916);  II(B, C, D, A, M[13], 21, 1309151649);
            II(A, B, C, D, M[ 4],  6, 4149444226);  II(D, A, B, C, M[11], 10, 3174756917);  II(C, D, A, B, M[ 2], 15, 718787259);   II(B, C, D, A, M[ 9], 21, 3951481745);

            buffer[0] += A;
            buffer[1] += B;
            buffer[2] += C;
            buffer[3] += D;
        };

        //  Padding data
        unsigned char PADDING[64] = {
            0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        //  Input buffer
        unsigned char input_buffer[64];
        //  Store the calculated hash
        unsigned char digest[16];
        //  Bit counter
        md5_block counter[2];
        //  MD5 states
        md5_block buffer[4];
};

}  //  end namespace mte

#endif
