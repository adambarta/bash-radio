#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <signal.h>

#include <dirent.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <queue.h>
#include <spead_api.h>

#define POSIX

#include "core/coreApi.h"
#include "matrixssl/matrixsslApi.h"


static int run = 1;

void handle_us()
{
  run = 0;
}

int register_signals()
{
  struct sigaction sa;

  sigfillset(&sa.sa_mask);
  sa.sa_handler   = handle_us;
  sa.sa_flags     = 0;

  if (sigaction(SIGINT, &sa, NULL) < 0)
    return -1;

  if (sigaction(SIGTERM, &sa, NULL) < 0)
    return -1;

  return 0;
}


unsigned char *get_resource_str(unsigned char *data)
{ 
  unsigned char *gloc=NULL, *hloc=NULL;
  
  if (data == NULL)
    return NULL;

  gloc = (unsigned char*) strstr((const char *)data, "GET");
  if (gloc){
    hloc = (unsigned char *)strstr((const char *)data, "HTTP");
    if (hloc){
      hloc--;
      hloc[0] = '\0';
      return gloc + 4;
    }
    return NULL; 
  }
  return NULL;
} 


void child_process(struct spead_client *cl, sslKeys_t *keys)
{
#define BUFF   1000 
//#define REPLY   "HTTP/1.1 200 OK\nServer: pshr\nConnection: Keep-Alive\nContent-Type: audio/mpeg\nCache-Control: no-cache\nPragma: no-cache\n\n"
#define REPLY   "HTTP/1.1 200 OK\nServer: pshr\nConnection: Keep-Alive\nContent-Type: text/plain\nCache-Control: no-cache\nPragma: no-cache\n\nhello world"
#define MODE_CONNECTING 0
#define MODE_SENDING    1  

  //int bytes, mode = MODE_CONNECTING;
  //unsigned char data[BUFF];
  
  unsigned char *data, *res;
  uint32 err, len ,rb, wb;
  ssl_t *ssl;
  //int fd = (-1), initial=1450-300;
  
  data = NULL;
  res  = NULL;
  ssl  = NULL;

  if (cl == NULL || keys == NULL){
#ifdef DEBUG
    fprintf(stderr, "%s: parameter error\n", __func__);
#endif
    exit(EXIT_FAILURE);
  }


#ifdef DEBUG
  fprintf(stderr, "%s: child created with fd[%d]\n", __func__, cl->c_fd);
#endif
  
  err = matrixSslNewServerSession(&ssl, keys, NULL, 0);
  if (err != PS_SUCCESS){
#ifdef DEBUG
    switch(err){
      case PS_ARG_FAIL:
        fprintf(stderr, "Bad input function parameter\n");
        break;
      case PS_FAILURE:
        fprintf(stderr, "Internal memory allocation failure\n");
        break;
    }
#endif
#if 0
    matrixSslDeleteKeys(keys);
    matrixSslClose();
#endif
    exit(EXIT_FAILURE);
  }

  while(run){

READ_STATE:
    len = matrixSslGetReadbuf(ssl, &data);
    if (rb < 0){
#ifdef DEBUG
      fprintf(stderr, "%s: matrixssl getreadbuf error\n", __func__);
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "%s: matrixssl getreadbuf rtn [%d]\n", __func__, len);
#endif

    rb = read(cl->c_fd, data, len);
    if (rb == 0){
#ifdef DEBUG
      fprintf(stderr, "%s: read EOF\n", __func__);
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    } else if (rb < 0){
#ifdef DEBUG
      fprintf(stderr, "%s: read error (%s)\n", __func__, strerror(errno));
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "%s: read rtn [%d]\n", __func__, rb);
#endif

    rb = matrixSslReceivedData(ssl, rb, &data, &len);
    if (rb < 0){
#ifdef DEBUG
      fprintf(stderr, "%s: matrixssl receiveddata error\n", __func__);
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    } else if (rb == 0){
#ifdef DEBUG
      fprintf(stderr, "%s: matrix ssl received 0 bytes (false start?)\n", __func__);
#endif
    } else if (rb > 0){
      switch(rb){
        case MATRIXSSL_REQUEST_SEND:
#ifdef DEBUG
          fprintf(stderr, "%s: RS req send\n", __func__);
#endif
          goto WRITE_STATE;
          break;
        case MATRIXSSL_REQUEST_RECV:
#ifdef DEBUG
          fprintf(stderr, "%s: RS req recv\n", __func__);
#endif  
          goto READ_STATE;
          break;
        case MATRIXSSL_HANDSHAKE_COMPLETE:
#ifdef DEBUG
          fprintf(stderr, "%s: RS handshake complete\n", __func__);
#endif
          goto READ_STATE;
          break;
        case MATRIXSSL_RECEIVED_ALERT:
#ifdef DEBUG
          fprintf(stderr, "%s: RS rec alert\n", __func__);
#endif
          break;
        case MATRIXSSL_APP_DATA:
#ifdef DEBUG
          fprintf(stderr, "%s: RS app data\n", __func__);
#endif

#ifdef DEBUG
          fprintf(stderr, "%s: RS got data [%s]\n", __func__, data);
#endif
          
          /*process client data here*/
  
          res = get_resource_str(data);
          if (res == NULL){
#ifdef DEBUG
            fprintf(stderr, "%s: NULL RESOURCE\n", __func__);        
#endif
            run = 0;
          }

#ifdef DEBUG
          fprintf(stderr, "%s: got resource [%s]\n", __func__, res); 
#endif
                    
          unsigned char *tbuf;
          int32 tbuflen;

          tbuflen = matrixSslGetWritebuf(ssl, &tbuf, strlen(REPLY));
          if (tbuflen < 0){
#ifdef DEBUG
            fprintf(stderr, "%s: matrixssl getwritebuf error\n", __func__);
#endif
            matrixSslDeleteSession(ssl);
            destroy_spead_client(cl);
            exit(EXIT_FAILURE);
          }
          
          strncpy((char *) tbuf, REPLY, tbuflen);
          
          if (matrixSslEncodeWritebuf(ssl, strlen((char *) tbuf)) < 0){
            matrixSslDeleteSession(ssl);
            destroy_spead_client(cl);
            exit(EXIT_FAILURE);
          }

          matrixSslEncodeClosureAlert(ssl);

          rb = matrixSslProcessedData(ssl, &data, &len);
          
#ifdef DEBUG
          fprintf(stderr, "%s: processed data rtn [%d]\n", __func__, rb);
#endif

          goto WRITE_STATE;
          
          break;
      }

    }

WRITE_STATE:
    len = matrixSslGetOutdata(ssl, &data);
    if (len < 0){
#ifdef DEBUG
      fprintf(stderr, "%s: matrixssl getoutdata error\n", __func__);
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "%s: getoutdata rtn [%d]\n", __func__, len);
#endif
    
    wb = write(cl->c_fd, data, len);
    if (wb == 0){
#ifdef DEBUG
      fprintf(stderr, "%s: write 0\n", __func__);
#endif
      goto READ_STATE;
    } else if (wb < 0){
#ifdef DEBUG
      fprintf(stderr, "%s: write error (%s)\n", __func__, strerror(errno));
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "%s: write rtn [%d]\n", __func__, wb);
#endif
    
    wb = matrixSslSentData(ssl, wb);
    if (wb < 0) {
      #ifdef DEBUG
      fprintf(stderr, "%s: matrixssl sentdata error\n", __func__);
#endif
      matrixSslDeleteSession(ssl);
      destroy_spead_client(cl);
      exit(EXIT_FAILURE);
    } else if (wb > 0){
      switch(wb){
        case MATRIXSSL_REQUEST_SEND:
#ifdef DEBUG
          fprintf(stderr, "%s: WS req send\n", __func__);
#endif
          goto WRITE_STATE;
          break;
        case MATRIXSSL_REQUEST_CLOSE:
#ifdef DEBUG
          fprintf(stderr, "%s: WS req close\n", __func__);
#endif  
          matrixSslDeleteSession(ssl);
          destroy_spead_client(cl);
          exit(EXIT_FAILURE);
          break;
        case MATRIXSSL_HANDSHAKE_COMPLETE:
#ifdef DEBUG
          fprintf(stderr, "%s: WS handshake complete\n", __func__);
#endif
          goto READ_STATE; /*note might need to jump to receiveddata*/
          break;
      }

    }

  }

  matrixSslDeleteSession(ssl);

#if 0
  const char *filename = "/home/adam/live_audio_streaming/tenc/agoria-scala_original_mixwww.mp3vip.org.mp3";

  //fd = open("/home/adam/build/lame-3.99.5/testcase.mp3", O_RDONLY);
  fd = open("/home/adam/live_audio_streaming/tenc/agoria-scala_original_mixwww.mp3vip.org.mp3", O_RDONLY);
  if (fd < 0){
#ifdef DEBUG
    fprintf(stderr, "%s: open error (%s)\n", __func__, strerror(errno));
#endif
    exit(EXIT_SUCCESS);
  }

  while((bytes = read_ssl(ssl, data, BUFF)) > 0){
#ifdef DEBUG
    fprintf(stderr, "%s: read [%d] [%s:%d] [%s]\n", __func__, bytes, get_client_address(cl), get_client_port(cl), data);
#endif
  }
#endif
  
#if 0
  while (run){

    switch (mode){
      
      case MODE_CONNECTING:
        
        bytes = read_ssl(ssl, data, BUFF);
        switch (bytes){
          case 0:
#ifdef DEBUG
            fprintf(stderr, "%s: read EOF client[%s:%d]\n", __func__, get_client_address(cl), get_client_port(cl));
#endif
            run = 0;
            break;

          case -1:
#ifdef DEBUG
            fprintf(stderr, "%s: read error client[%s:%d] (%s)\n", __func__, get_client_address(cl), get_client_port(cl), strerror(errno));
#endif
            run = 0;
            break;
        }

#ifdef DEBUG
        fprintf(stderr, "%s: [%s:%d] [%s]\n", __func__, get_client_address(cl), get_client_port(cl), data);
#endif
        
        res = get_resource_str(data);
        if (res == NULL){
#ifdef DEBUG
          fprintf(stderr, "%s: NULL RESOURCE\n", __func__);        
#endif
          run = 0;
        }
  
#ifdef DEBUG
       fprintf(stderr, "%s: got resource [%s]\n", __func__, res); 
#endif
       
       if (strncmp(res, "/sound", 6) == 0){
         bytes = write_ssl(ssl, REPLY, sizeof(REPLY));
         switch(bytes){
           case -1:
#ifdef DEBUG
             fprintf(stderr, "%s: write error client[%s:%d] (%s)\n", __func__, get_client_address(cl), get_client_port(cl), strerror(errno));
#endif
             run = 0;
             break;
         }
         mode = MODE_SENDING;
       } else {
         run = 0;
       }
       break;


      case MODE_SENDING:
#if 0
        bytes = sendfile(cl->c_fd, fd, NULL, BUFF+initial);
        if (bytes == 0){
          close(fd);
#if 0
          fd = open("/srv/beats/Meditations On Afrocentrism EP/03 Down The Line (It Takes A Number).mp3", O_RDONLY);
          if (fd < 0){
#ifdef DEBUG
            fprintf(stderr, "%s: open error (%s)\n", __func__, strerror(errno));
#endif
            exit(EXIT_SUCCESS);
          }
#endif
        } else if (bytes < 0){
#ifdef DEBUG
          fprintf(stderr, "%s: sendfile error client[%s:%d] (%s)\n", __func__, get_client_address(cl), get_client_port(cl), strerror(errno));
#endif
          exit(EXIT_SUCCESS);
        }

        if (initial > 1){
          initial--;
        } 

#if 0
def DEBUG
        fprintf(stderr, "%s: [%s:%d] sent %d bytes\n", __func__, get_client_address(cl), get_client_port(cl), bytes);
#endif
        usleep(9766/initial);
#endif    
        run = 0;
        break;
    }
  }
#endif

#ifdef DEBUG
  fprintf(stderr, "%s: child[%d] ending\n", __func__, getpid());
#endif

#if 0
  if (fd > 0){
    close(fd);
  } 
#endif

  destroy_spead_client(cl);
  //shutdown(cl->c_fd, SHUT_RDWR);
  exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[])
{
  struct spead_socket *x;
  struct spead_client *c;

  sslKeys_t *keys;
  int32 err;
  
  if (register_signals() < 0)
    return 1;

  x = create_tcp_socket(NULL, "3333");
  if (x == NULL)
    return 1;

  if (bind_spead_socket(x) < 0){
    destroy_spead_socket(x);
    destroy_shared_mem();
    return 1;
  }

  if (listen_spead_socket(x) < 0) {
    destroy_spead_socket(x);
    destroy_shared_mem();
    return 1;
  }

  err = matrixSslOpen();
  if (err != PS_SUCCESS){
#ifdef DEBUG
    fprintf(stderr, "%s: error setting up matrixssl\n", __func__);
#endif
    destroy_spead_socket(x);
    destroy_shared_mem();
    return 1;
  }

  err = matrixSslNewKeys(&keys);
  if (err != PS_SUCCESS){
#ifdef DEBUG
    fprintf(stderr, "%s: error allocationg matrixssl keys\n", __func__);
#endif
    destroy_spead_socket(x);
    destroy_shared_mem();
    matrixSslClose();
    return 1;
  }

  err = matrixSslLoadRsaKeys(keys, "./certs/server.crt", "./certs/server.key", NULL, NULL);
  if (err != PS_SUCCESS){
#ifdef DEBUG
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
#endif
    destroy_spead_socket(x);
    destroy_shared_mem();
    matrixSslDeleteKeys(keys);
    matrixSslClose();
    return 1;
  }


  while (run){
  
    c = accept_spead_socket(x);
    if (c){
      
      switch(fork()){
        case -1:
#ifdef DEBUG
          fprintf(stderr, "%s: fork err (%s)\n", __func__, strerror(errno));
#endif
          break;
        
        /*child*/
        case 0:
          /*child process takes over the accept object*/
          child_process(c, keys);
          exit(EXIT_SUCCESS);
          break;

        /*parent*/
        default:
#ifdef DEBUG
          fprintf(stderr, "%s: close the child fd on the parent\n", __func__);
#endif
          /*server closes the file descriptor and frees the object but doesn't 
            shutdown the connection from accept*/
          close(c->c_fd);
          free(c);
          break;
      }
    }

  }

  destroy_spead_socket(x);
  destroy_shared_mem();
  matrixSslDeleteKeys(keys);
  matrixSslClose();

  return 0;
}
