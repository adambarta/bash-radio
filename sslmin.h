#ifndef SSL_MIN_H
#define SSL_MIN_H

#include <openssl/ssl.h>

int read_ssl(SSL *ssl, void *buf, int size);
int write_ssl(SSL *ssl, void *buf, int size);

SSL_CTX *setup_tls();
void shutdown_tls(SSL_CTX *tlsctx);

SSL *accept_client_ssl(SSL_CTX *tlsctx, int cfd);
void shutdown_client_ssl(SSL *ssl);

#endif
