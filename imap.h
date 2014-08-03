#ifndef IMAP_H
#define IMAP_H

#include <openssl/ssl.h>
#include <functional>
#include <string>

namespace imappp
{

class status;

/**
 * An imap connection to a server, using SSL.
 */
class imap
{
public:
	typedef std::function<bool(const status&)> status_callback;

	typedef std::function<void(unsigned int)> receive_message_callback;

	typedef std::function<void(bool)> connection_callback;

	/**
	 * Instantiate an IMAP SSL connection to host.
	 * @param host the host to connect to.
	 */
	imap(const char* host, bool verbose=false);

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
	 * Try to connect to the server.
	 * @return true in case of success
	 */
	bool connect();

	/**
	 * Log on the server.
	 * @param login User to log into
	 * @param password Password to use
	 * @return true in case of success
	 */
	bool login(const char *login, const char *password);

	/**
	 * Selects a mailbox for use with subsequent commands.
	 */
	void select(const char *mailbox);

	/**
	 * Go into idle mode.
	 */
	void idle();

	/**
	 * Stop idle mode.
	 */
	void stop_idle();

	/**
	 * Tells the server the connection will be closed soon.
	 */
	void logout();

	/**
	 * Wait for more data. Useful when idling.
	 *
	 * @return false if the connection is closed.
	 */
	bool wait();

	/**
	 * Return the number of unread messages.
	 */
	unsigned int unread_count() const;

	/**
	 * Return the number of messages.
	 */
	unsigned int message_count() const;

	/**
	 * Sets the method to call when receiving the message read count.
	 */
	void set_message_callback(receive_message_callback callback);

	/**
	 * Sets the method to call when receiving the message read count.
	 */
	void set_connection_callback(connection_callback callback);

	void stop_reception();
private:
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
	 * Try to send the data in format/ap (like sprintf) with len bytes.
	 *
	 * If success, return 0. If failed, return the needed number of bytes.
	 */
	int vsendf(const char *format, va_list ap)
		__attribute__((format(printf, 2, 0)));

	/**
	 * Restart the connection and the handshake. Returns 0 on success.
	 */
	int handshake(bool ignore_errors);

	/**
	 * Reset the connection and try to make it into a workable state.
	 * Returns 0 on success.
	 */
	int reset(bool ignore_errors);

	/**
	 * Printf based send to server.
	 */
	void sendf(const char *format, ...)
		__attribute__((format(printf, 2, 3)));

	/**
	 * Receive the incoming buffer.
	 * @param func function to call for each received message
	 * @return size in bytes of the received data
	 */
	int receive(status_callback func = nullptr);

	/**
	 * Default actions for the specified status.
	 */
	bool default_callback(const status& s);

	/**
	 * Pre process messages.
	 */
	void pre_process(const status& s);

	/**
	 * SSL context.
	 */
	SSL_CTX *ctx_;

	/**
	 * SSL connection.
	 */
	BIO *connection_;

	/**
	 * Identifier of the next command to send.
	 */
	unsigned int tag_;

	/**
	 * If true, print all text-based comminucation to stdout.
	 */
	bool verbose_;

	/**
	 * True if we currently are in idle mode.
	 */
	bool idle_;

	/**
	 * True if we're currently logging out.
	 */
	bool logout_;


	/**
	 * Number of recent messages.
	 */
	unsigned int recent_count_;

	/**
	 * Number of messages in the mailbox.
	 */
	unsigned int message_count_;

	/**
	 * Number of unread messages.
	 */
	unsigned int unread_count_;

	/**
	 * If true, we are currently not sure of the count of unread messages.
	 */
	bool need_refresh_;

	/**
	 * If set, this is called when the number of messages change.
	 */
	receive_message_callback message_callback_;

	/**
	 * If set, this is called when the connection or disconnection status changes.
	 */
	connection_callback connection_callback_;

	/**
	 * Some file that can be monitored to detect suspend events.
	 */
	int suspend_fd_;

	/**
	 * A file descriptor used to detect stop events.
	 */
	int stop_fd_;

	/**
	 * Write to this pipe to stop the receive loop.
	 */
	int stop_pipe_fd_;
};

}

#endif
