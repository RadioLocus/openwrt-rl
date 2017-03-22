#include <polarssl/md.h>
#include <stdint.h>

#define crypto_auth_BYTES 32
#define OTP_BYTES 8

int DIGITS_POWER[] = {1,10,100,1000,10000,100000,1000000,10000000,100000000};

int Truncate (unsigned char *hmac, int N)
{
    // uint64_t O = least_four_significant_bits_hmac;
    int hmacLen = 20;
    int O = (hmac[hmacLen -1] & 0x0f);
    int bin_code = ((hmac[O] & 0x7f) << 24) | ((hmac[O+1] & 0xff) << 16) | ((hmac[O+2] & 0xff) << 8) | ((hmac[O+3] & 0xff));
    int token = bin_code % DIGITS_POWER[N];
    return token;
}


// set he secret string to be exactly crypto_auth_BYTES bytes
int setSecretStr(unsigned char fixedSecret[OTP_BYTES + 1],
const unsigned char *secret) {
    int16_t sLen = 0;

    if (secret == NULL) {
        //assert(LIB_NAME "secret string must not be NULL" && (false ||
        //Otp_TestMode));
        return 1;
    }
    secret = htonl(secret);

    sLen = strlen((const char *)secret);
    if (sLen > OTP_BYTES) sLen = OTP_BYTES;
    memset(fixedSecret, 0, OTP_BYTES +1);
    memcpy(&(fixedSecret[OTP_BYTES - sLen]),
             secret, sLen);
    fixedSecret[OTP_BYTES] = 0;
    return 0;
    }

int Crypto_CalcHmac(const unsigned char *key, int16_t keyLen, const unsigned char *input, size_t inputLen, unsigned char *output) {
      int16_t ret = 0;
        const md_info_t *mdInfo = NULL;
          mdInfo = md_info_from_type(POLARSSL_MD_SHA1);
            if ((ret = md_hmac(mdInfo, key, keyLen, input, inputLen, output)) != 0) {
                    printf("Error while calculating md_hmac, errIdx %d", ret);
                    return 1;
             }
             return 0;
}

int generateHMAC(char *key, char* otp) {
    int16_t mask = 0xf, offset = 0, len = crypto_auth_BYTES, sLen = 0;
    int32_t code;
    unsigned char digest[crypto_auth_BYTES], secret[crypto_auth_BYTES + 1];
    char fmt[10];
    int i = 0;
    setSecretStr(secret, otp);
    Crypto_CalcHmac(key, strlen(key), (unsigned char *)secret, 8, digest);
    return digest;

}

int generateHOTP(char *K, long C) {
    C = htonl(C);
    unsigned char secret[OTP_BYTES];
    memset(secret, 0, OTP_BYTES);
    memcpy(&(secret), (unsigned char *)&C, sizeof(C));
    unsigned char *hmac = generateHMAC(K, secret);
    return hmac;
}


int generateTOTP(char *K, int N) {
    long timeInterval = ((uint64_t) time (NULL))/30;

    unsigned char *hmac = generateHOTP(K, timeInterval);
    return Truncate (hmac, N);

}

int generateTOTPUsingTimestamp(char *K, int N, long timestamp) {
    long timeInterval = timestamp/30;
    unsigned char *hmac = generateHOTP(K, timeInterval);
    return Truncate (hmac, N);
}
