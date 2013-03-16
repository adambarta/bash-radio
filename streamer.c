/* example.c: Demonstration of the libshout API.
 * $Id: example.c 8144 2004-10-30 01:24:47Z brendan $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shout/shout.h>

int main(int argc, char *argv[])
{
	shout_t *shout;
	char buff[1024];
	long read, ret, total;

  if (argc < 4){
    fprintf(stderr, "usage %s [0|1] hostname port password mount\n\t0 ogg stream type\n\t1 mp3 stream type\n", argv[0]);
    return 1;
  }

	shout_init();

	if (!(shout = shout_new())) {
		fprintf(stderr, "Could not allocate shout_t\n");
		return 1;
	}

	if (shout_set_host(shout, argv[2]) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting hostname: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting protocol: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_port(shout, atoi(argv[3])) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting port: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_password(shout, argv[4]) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting password: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_mount(shout, argv[5]) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting mount: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting user: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_format(shout, atoi(argv[1])) != SHOUTERR_SUCCESS) {
		fprintf(stderr, "Error setting user: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_open(shout) == SHOUTERR_SUCCESS) {
		fprintf(stderr, "Connected to server...\n");
		total = 0;
		while (1) {
			read = fread(buff, 1, sizeof(buff), stdin);
			total = total + read;

			if (read > 0) {
				ret = shout_send(shout, buff, read);
				if (ret != SHOUTERR_SUCCESS) {
					fprintf(stderr, "DEBUG: Send error: %s\n", shout_get_error(shout));
					//break;
				}
			} else {
				//break;
			}

			shout_sync(shout);
		}
	} else {
		fprintf(stderr, "Error connecting: %s\n", shout_get_error(shout));
	}

	shout_close(shout);

	shout_shutdown();

	return 0;
}
