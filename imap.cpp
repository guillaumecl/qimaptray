#include "imap.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#include <openssl/err.h>
#include <stdarg.h>
#include <string.h>

#include "status.h"

#define TAG "%010d "

using namespace imappp;

imap::imap(const char *host, bool verbose) :
	tag_(0),
	verbose_(verbose),
	idle_(false),
	logout_(false)
{
	/* TODO handle exceptions during the setup (destructor won't be called) */
	BIO *sbio;
	int port = 993;

	/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();

	ctx_ = SSL_CTX_new(SSLv23_method());
#ifdef SSL_MODE_RELEASE_BUFFERS
	SSL_CTX_set_mode(ctx_, SSL_MODE_RELEASE_BUFFERS);
#endif
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
}

imap::~imap()
{
	/* TODO clean this. Ensure this works even if the connection is already
	   closed, etc.
	*/
	SSL_shutdown(connection_);
	SSL_CTX_free(ctx_);
	close(sock_);
}

void imap::logout()
{
	if (idle_)
	{
		stop_idle();
	}

	logout_ = true;

	sendf("LOGOUT");
	receive();
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

int imap::try_sendf(int length, const char *format, va_list ap)
{
	char buffer[length];

	int l = sprintf(buffer, TAG, tag_);

	int len = vsnprintf(buffer + l, sizeof(buffer) - l, format, ap);
	len += l;
	if (len + 2 > length)
	{
		return len + 3;
	}
	else
	{
		buffer[len] = '\r';
		buffer[len+1] = '\n';
		buffer[len+2] = 0;
		if (verbose_)
		{
			printf("%s", buffer);
		}
		SSL_write(connection_, buffer, len+2);
		return 0;
	}
}

void imap::sendf(const char *format, ...)
{
	va_list ap;
	unsigned int len;

	va_start(ap, format);

	len = try_sendf(1024, format, ap);
	if (len > 0)
	{
		fprintf(stderr, "warning: send buffer too small: needed:%d.\n", len);
		try_sendf(len, format, ap);
	}
	tag_++;

	va_end(ap);
}

void imap::error(const char *string)
{
	fprintf(stderr,"%s\n",string);
	throw string;
}

void imap::ssl_error(const char* string)
{
	if (not logout_)
	{
		/*
		 * During logout, some servers disconnect early. We really don't want
		 * to throw since we're probably in the destructor.
		 */
		fprintf(stderr,"%s\n",string);
		ERR_print_errors_fp(stderr);
		throw string;
	}
}

int imap::receive(std::function<void(const status&)> iter)
{
	char buffer[2048];

	int ret = SSL_read(connection_, buffer, sizeof(buffer));
	switch(SSL_get_error(connection_, ret))
	{
	case SSL_ERROR_NONE:
		break;
	case SSL_ERROR_ZERO_RETURN:
		ret = 0;
		break;
	case SSL_ERROR_SYSCALL:
		ssl_error("SSL Error: Premature close");
		break;
	default:
		ssl_error("SSL read problem");
	}
	buffer[ret] = 0;
	if (verbose_)
	{
		printf("%s", buffer);
	}

	status::parse(buffer, iter);

	return ret;
}

bool imap::login(const char *login, const char *password)
{
	bool success = true;
	sendf("LOGIN %s %s", login, password);

	receive([&success] (const status& s)
			{
				if (s.response() != ok)
				{
					success = false;
				}
			});

	return success;
}

void imap::select(const char *mailbox)
{
	sendf("SELECT %s", mailbox);
	receive();
}

void imap::idle()
{
	sendf("IDLE");

	idle_ = true;
	receive();
}

void imap::stop_idle()
{
	SSL_write(connection_, "DONE\r\n", 6);
	receive();

	idle_ = false;
}


bool imap::wait()
{
	return receive() > 0;
}
