#include <string>
#include <functional>

namespace imappp
{

enum response
{
	untagged,
	ok,
	no,
	bad,
};

enum command
{
	unknown,
	expunge,
	exists,
	recent,
	fetch,
	search,
};

class status
{
public:
	status();

	unsigned int tag() const;
	unsigned int num() const;

	const std::string& message() const;
	const std::string& error() const;

	enum response response() const;

	enum command command() const;

	bool tagged() const;

	static void parse(const char *buffer, std::function<void(const status&)> visitor);
private:

	unsigned int tag_;

	unsigned int num_;

	std::string message_;

	enum response response_;

	enum command command_;
};

inline unsigned int status::tag() const
{
	return tag_;
}

inline bool status::tagged() const
{
	return tag_ != static_cast<unsigned int>(-1);
}

inline const std::string& status::message() const
{
	return message_;
}

inline enum response status::response() const
{
	return response_;
}

inline enum command status::command() const
{
	return command_;
}

inline unsigned int status::num() const
{
	return num_;
}

}
