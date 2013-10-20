#include "imap.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#include <openssl/err.h>

using namespace imappp;

imap::imap(const char *host)
{
	/* TODO handle exceptions during the setup (destructor won't be called) */
	memset(&id, '0', sizeof(id));

	BIO *sbio;
	int port = 993;

	/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();

	ctx_ = SSL_CTX_new(SSLv23_method());

	tcp_connect(host, port);

	connection_ = SSL_new(ctx_);
	sbio = BIO_new_socket(sock_, BIO_NOCLOSE);
	SSL_set_bio(connection_, sbio, sbio);

	if(SSL_connect(connection_) <= 0)
	{
		ssl_error("SSL connect error");
	}

	/**
	 * Receive the initial dialog from the server.
	 */
	receive();
	printf("%s\n", receive_buffer);
}

imap::~imap()
{
	/* TODO clean this. Ensure this works even if the connection is already
	   closed, etc.
	*/
	if (SSL_shutdown(connection_) != 1)
	{
		fprintf(stderr, "SSL Shutdown failed\n");
		ERR_print_errors_fp(stderr);
		// don't call ssl_error because we don't want to throw anything here.
	}
	SSL_free(connection_);
	SSL_CTX_free(ctx_);
	close(sock_);
}


void imap::tcp_connect(const char *host, int port)
{
	struct hostent *hp;
	struct sockaddr_in addr;

	if(!(hp=gethostbyname(host)))
	{
		error("Couldn't resolve host");
	}

	memset(&addr,0,sizeof(addr));

	addr.sin_addr = *(struct in_addr*) hp->h_addr_list[0];
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if((sock_ = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP))<0)
	{
		error("Couldn't create socket");
	}

	if(connect(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		error("Couldn't connect socket");
	}
}

void imap::send_raw(const char *buffer)
{
	send_id();

	SSL_write(connection_, buffer, strlen(buffer));

	SSL_write(connection_, "\n", 1);
}

int imap::receive(char *buffer, int buffsize)
{
	int r = SSL_read(connection_, buffer, buffsize);
	switch(SSL_get_error(connection_, r))
	{
	case SSL_ERROR_NONE:
		break;
	case SSL_ERROR_ZERO_RETURN:
		r = 0;
		break;
	case SSL_ERROR_SYSCALL:
		ssl_error("SSL Error: Premature close");
		break;
	default:
		ssl_error("SSL read problem");
	}
	buffer[r] = 0;
	return r;
}

void imap::error(const char *string)
{
	fprintf(stderr,"%s\n",string);
	throw string;
}

void imap::ssl_error(const char* string)
{
	fprintf(stderr,"%s\n",string);
	ERR_print_errors_fp(stderr);
	throw string;
}

void imap::send_id()
{
	SSL_write(connection_, id, sizeof(id));
	SSL_write(connection_, " ", 1);

	int i = sizeof(id) - 1;
	while (i >= 0)
	{
		id[i]++;
		if (id[i] < '9')
		{
			break;
		}
		else
		{
			id[i] = '0';
		}
		i--;
	}
}

int imap::receive()
{
	return receive(receive_buffer, sizeof(receive_buffer));
}

void imap::login(const char *login, const char *password)
{
	send_id();

	SSL_write(connection_, "LOGIN ", 6);

	SSL_write(connection_, login, strlen(login));
	SSL_write(connection_, " ", 1);

	SSL_write(connection_, password, strlen(password));
	SSL_write(connection_, "\n", 1);

	receive();

	printf("%s\n", receive_buffer);
}
