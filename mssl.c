#include "stdio.h"

#define POSIX

#include "core/coreApi.h"
#include "matrixssl/matrixsslApi.h"

int main(int argc, char *argv[])
{
  sslKeys_t *keys;
  ssl_t *ssl;

  int err;

  err = matrixSslOpen();
  if (err != PS_SUCCESS){
    fprintf(stderr, "%s: error setting up matrixssl\n", __func__);
    return -1;
  }

  err = matrixSslNewKeys(&keys);
  if (err != PS_SUCCESS){
    fprintf(stderr, "%s: error allocationg matrixssl keys\n", __func__);
    matrixSslClose();
    return -1;
  }

  err = matrixSslLoadRsaKeys(keys, "./certs/server.crt", "./certs/server.key", NULL, NULL);
  if (err != PS_SUCCESS){
    switch(err){
      case PS_CERT_AUTH_FAIL:
        fprintf(stderr, "Certificate or chain did not self-authenticate or private key could not authenticate certificate\n");
        break;
      case PS_PLATFORM_FAIL:
        fprintf(stderr, "Error locating or opening an input file\n");
        break;
      case PS_ARG_FAIL:
        fprintf(stderr, "Bad input function parameter\n");
        break;
      case PS_MEM_FAIL:
        fprintf(stderr, "Internal memory allocation failure\n");
        break;
      case PS_PARSE_FAIL:
        fprintf(stderr, "Error parsing certificate or private key buffer\n");
        break;
      case PS_FAILURE:
        fprintf(stderr, "Password protected decoding failed. Likey incorrect password provided\n");
        break;
      case PS_UNSUPPORTED_FAIL:
        fprintf(stderr, "Unsupported key algorithm in certificate material\n");
        break;
    }
    matrixSslDeleteKeys(keys);
    matrixSslClose();
    return -1;
  }
  
  err = matrixSslNewServerSession(&ssl, keys, NULL, 0);
  if (err != PS_SUCCESS){
    switch(err){
      case PS_ARG_FAIL:
        fprintf(stderr, "Bad input function parameter\n");
        break;
      case PS_FAILURE:
        fprintf(stderr, "Internal memory allocation failure\n");
        break;
    }
    matrixSslDeleteKeys(keys);
    matrixSslClose();
    return -1;
  }


  matrixSslDeleteSession(ssl);
  matrixSslDeleteKeys(keys);
  matrixSslClose();

  return 0;
}
