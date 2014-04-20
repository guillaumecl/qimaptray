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

#include <poll.h>

#include "status.h"

#define TAG "%010d "

using namespace imappp;

imap::imap(const char *host, bool verbose) :
	tag_(0),
	verbose_(verbose),
	idle_(false),
	logout_(false),
	need_refresh_(false)
{
	SSL *ssl;
	int ret;
	int port = 993;
	/* TODO handle exceptions during the setup (destructor won't be called) */
	/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();

	ERR_load_crypto_strings();
	ERR_load_SSL_strings();
	OpenSSL_add_all_algorithms();


	ctx_ = SSL_CTX_new(SSLv23_client_method());

	connection_ = BIO_new_ssl_connect(ctx_);
	BIO_get_ssl(connection_, &ssl);

	if (!ssl)
	{
		fprintf(stderr, "Cannot get a SSL object.");
		return;
	}

	/* Don't want any retries */
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	BIO_set_conn_hostname(connection_, host);
	BIO_set_conn_int_port(connection_, &port);

	BIO_set_nbio(connection_, 1);

	do
	{
		ret = BIO_do_connect(connection_);
	} while (ret <= 0 && BIO_should_retry(connection_));

	if (ret <= 0)
	{
		fprintf(stderr, "Error connecting to server: %d\n", ret);
		ERR_print_errors_fp(stderr);
		return;
	}

	do
	{
		ret = BIO_do_handshake(connection_);
	} while (ret <= 0 && BIO_should_retry(connection_));

	if (ret <= 0)
	{
		fprintf(stderr, "Error doing SS handshake: %d\n", ret);
		ERR_print_errors_fp(stderr);
		return;
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
	BIO_free_all(connection_);
	SSL_CTX_free(ctx_);
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

int imap::try_sendf(int length, const char *format, va_list ap)
{
	char buffer[length];
	int ret;

	int l = sprintf(buffer, TAG, tag_);

	int len = vsnprintf(buffer + l, sizeof(buffer) - l, format, ap);
	len += l;
	if (len + 2 > length)
	{
		return len + 3;
	}
	else
	{
		if (verbose_)
		{
			printf("%s\n", buffer);
		}
		buffer[len] = '\r';
		buffer[len+1] = '\n';
		buffer[len+2] = 0;
		do
		{
			ret = BIO_write(connection_, buffer, len+2);
		} while (ret < len+2 and BIO_should_retry(connection_));
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

int imap::receive(status_callback callback)
{
	char buffer[2048];

	int ret;

	struct pollfd p;

	while(true)
	{
		BIO_get_fd(connection_, &p.fd);
		p.events = POLLIN | POLLRDHUP | POLLNVAL;
		fprintf(stderr, "Polling %d\n", p.fd);
		ret = poll(&p, 1, 10*60*1000);
		if (ret == 0)
		{
			fprintf(stderr, "Lost connection : %d\n", ret);
			BIO *bio = BIO_new_ssl_connect(ctx_);
			int port = BIO_get_conn_int_port(connection_);

			BIO_set_conn_hostname(bio, BIO_get_conn_hostname(connection_));
			BIO_set_conn_int_port(bio, &port);

			BIO_set_nbio(bio, 1);

			std::swap(bio, connection_);
			BIO_free_all(bio);

			return ret;
		}
		ret = BIO_read(connection_, buffer, sizeof(buffer));
		if (ret > 0 or not BIO_should_retry(connection_))
		{
			break;
		}
	}

	if (ret <= 0)
	{
		return ret;
	}

	buffer[ret] = 0;
	if (verbose_)
	{
		printf("%s", buffer);
	}

	status::parse(
		buffer,
		[this, &callback] (const status& s)
		{
			pre_process(s);
			if (callback and not callback(s))
			{
				default_callback(s);
			}
		});

	if (need_refresh_)
	{
		need_refresh_ = false;
		bool was_idle = idle_;
		if (was_idle)
		{
			stop_idle();
		}
		sendf("SEARCH UNSEEN");
		if (was_idle)
		{
			sendf("IDLE");
			idle_ = true;
		}
	}

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
				return true;
			});
	return success;
}

bool imap::default_callback(const status& /*s*/)
{
	return true;
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
	BIO_write(connection_, "DONE\r\n", 6);
	receive();

	idle_ = false;
}


bool imap::wait()
{
	return receive() > 0;
}

void imap::pre_process(const status& s)
{
	switch(s.command())
	{
	case exists:
		message_count_ = s.num();
		need_refresh_ = true;
		break;
	case recent:
		recent_count_ = s.num();
		break;
	case expunge:
		message_count_--;
		need_refresh_ = true;
		break;
	case fetch:
		need_refresh_ = true;
		break;
	case search:
		unread_count_ = s.num();
		if (message_callback_)
		{
			message_callback_(s.num());
		}
		break;
	case unknown:
		// Nothing to do if this is not a command to handle.
		break;
	}
}


unsigned int imap::unread_count() const
{
	return unread_count_;
}

unsigned int imap::message_count() const
{
	return message_count_;
}

void imap::set_message_callback(receive_message_callback callback)
{
	message_callback_ = callback;
}
