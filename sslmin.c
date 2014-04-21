
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#define TLS_CERT  "./certs/server.crt"
#define TLS_KEY   "./certs/server.key"


int read_ssl(SSL *ssl, void *buf, int size)
{
  return SSL_read(ssl, buf, size);
}

int write_ssl(SSL *ssl, void *buf, int size)
{
  return SSL_write(ssl, buf, size);
}

SSL_CTX *setup_tls()
{
  SSL_CTX *tlsctx;

  SSL_library_init();
  SSL_load_error_strings();

  tlsctx = SSL_CTX_new(TLSv1_server_method());
  if (tlsctx == NULL){
#ifdef DEBUG
    fprintf(stderr, "wss: SSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
#endif
    return NULL;
  }

#ifdef DEBUG
  fprintf(stderr, "%s: tlsctx (%p)\n", __func__, tlsctx);
#endif

  SSL_CTX_set_options(tlsctx, SSL_OP_SINGLE_DH_USE);

  if (!SSL_CTX_use_certificate_file(tlsctx, TLS_CERT, SSL_FILETYPE_PEM)){
#ifdef DEBUG
    fprintf(stderr, "%s: SSL error: %s\n", __func__, ERR_error_string(ERR_get_error(), NULL));
#endif
    SSL_CTX_free(tlsctx);
    return NULL;
  }

  if (!SSL_CTX_use_PrivateKey_file(tlsctx, TLS_KEY, SSL_FILETYPE_PEM)) {
#ifdef DEBUG
    fprintf(stderr, "%s: SSL error: %s\n", __func__, ERR_error_string(ERR_get_error(), NULL));
#endif
    SSL_CTX_free(tlsctx);
    return NULL;
  }

  //SSL_CTX_set_session_cache_mode(tlsctx, SSL_SESS_CACHE_OFF);
  
  return tlsctx;
}

void shutdown_tls(SSL_CTX *tlsctx)
{
  SSL_CTX_free(tlsctx);
  COMP_zlib_cleanup();
  ERR_remove_state(0);
  ENGINE_cleanup();
  CONF_modules_unload(1);
  ERR_free_strings();
  EVP_cleanup();
  CRYPTO_cleanup_all_ex_data();
}

SSL *accept_client_ssl(SSL_CTX *tlsctx, int cfd)
{
  SSL *ssl;
  ssl = SSL_new(tlsctx);
  if (ssl == NULL){ 
#ifdef DEBUG
    fprintf(stderr, "%s: SSL error: %s\n", __func__, ERR_error_string(ERR_get_error(), NULL));
#endif
    return NULL;
  }
  SSL_set_fd(ssl, cfd);
  SSL_accept(ssl);
#ifdef DEBUG
  fprintf(stderr, "%s: client fd: %d ssl (%p)\n", __func__, cfd ,ssl); 
#endif
  return ssl;
}

void shutdown_client_ssl(SSL *ssl)
{
  if (ssl){
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
}
