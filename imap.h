#ifndef IMAP_H
#define IMAP_H

#include <openssl/ssl.h>

namespace imappp
{


/**
 * An imap connection to a server, using SSL.
 */
class imap
{
public:
	/**
	 * Instantiate an IMAP SSL connection to host.
	 * @param host the host to connect to.
	 */
	imap(const char* host);

	/**
	 * Prevent copy.
	 */
	imap(const imap&) = delete;

	/**
	 * Prevent copy.
	 */
	imap(imap&&) = delete;

	/**
	 * Destructor.
	 */
	~imap();

	/**
	 * Prevent affectation.
	 */
	imap& operator=(const imap&) = delete;

	/**
	 * Prevent affectation.
	 */
	imap& operator=(imap&&) = delete;


	/**
	 * Log on the server.
	 * @param login User to log into
	 * @param password Password to use
	 */
	void login(const char *login, const char *password);

	/**
	 * Send a raw command to the server.
	 * @param buffer string to send.
	 */
	void send_raw(const char *buffer);

	/**
	 * Receive the incoming buffer.
	 * @param buffer reception buffer
	 * @param buffsize sisze of the reception buffer
	 * @return size in bytes of the received data
	 */
	int receive(char *buffer, int buffsize);

private:
	/**
	 * Connects to the specified host and port.
	 * @param host the hostname (either an address or a host to resolve)
	 * @param port the port to create the connection on.
	 */
	void tcp_connect(const char *host, int port);

	/**
	 * Logs errors.
	 * @param string the error
	 */
	void error(const char *string);

	/**
	 * Logs error about ssl.
	 * @param string the error
	 */
	void ssl_error(const char* string);

	/**
	 * Send the id of the current connection.
	 */
	void send_id();

	/**
	 * Receive data in the internal buffer.
	 * @return length of the received data.
	 */
	int receive();

	/**
	 * SSL context.
	 */
	SSL_CTX *ctx_;

	/**
	 * SSL connection.
	 */
	SSL *connection_;

	/**
	 * Underlying socket fd (don't recv on that, but maybe can be used for select).
	 */
	int sock_;

	/**
	 * Identifier of the next command to send.
	 */
	char id[10];

	/**
	 * Reception buffer.
	 */
	char receive_buffer[4096];
};

}

#endif
