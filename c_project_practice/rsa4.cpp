#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

//加密
int my_encrypt(const char *input, int input_len,  char *output, int *output_len, const char *pri_key_fn)
{
        RSA  *p_rsa = NULL;
        FILE *file = NULL;
        int ret = 0;

        if((file = fopen(pri_key_fn, "rb")) == NULL)
        {
                ret = -1;
                goto End;
        }

        if((p_rsa = PEM_read_RSAPrivateKey(file, NULL,NULL,NULL )) == NULL)
        {
                ret = -2;
                goto End;
        }

        if((*output_len = RSA_private_encrypt(input_len, (unsigned char*)input, (unsigned char*)output, p_rsa, RSA_PKCS1_PADDING)) < 0)
        {
                ret = -4;
                goto End;
        }

End:
        if(p_rsa != NULL)
                RSA_free(p_rsa);
        if(file != NULL)
                fclose(file);

        return ret;
}

//解密
int my_decrypt(const char *input, int input_len,  char *output, int *output_len, const char *pri_key_fn)
{
        RSA  *p_rsa = NULL;
        FILE *file = NULL;
        int ret = 0;

        file = fopen(pri_key_fn, "rb");
        if(!file)
        {
                ret = -1;
                goto End;
        }

        if((p_rsa = PEM_read_RSA_PUBKEY(file, NULL,NULL,NULL )) == NULL)
        {
                ret = -2;
                goto End;
        }

        if((*output_len=RSA_public_decrypt(input_len, (unsigned char*)input, (unsigned char*)output, p_rsa, RSA_PKCS1_PADDING)) < 0)
        {
                ret = -3;
                goto End;
        }
End:
        if(p_rsa != NULL)
                RSA_free(p_rsa);
        if(file != NULL)
                fclose(file);

        return ret;
}

int main(int argc, char**argv)
{
    char src[256];
    char dst[256];
    int src_len;
    int dst_len;
    int ret;
    FILE *f;

    src_len = fread(src, 1, 256, stdin);

    if(argv[1][0] == 'e') {
            ret = my_encrypt(src, src_len,  dst, &dst_len, argv[2]);
    }else {
            ret = my_decrypt(src, src_len,  dst, &dst_len, argv[2]);
    }

    if(ret) {
            fprintf(stderr, "Error\n");
    }
    fwrite(dst,1,dst_len,stdout);
    return ret;
}