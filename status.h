#include <string>
#include <functional>

namespace imappp
{

enum response
{
	ok,
	no,
	bad,
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

	bool tagged() const;

	static void parse(const char *buffer, std::function<void(const status&)> visitor);
private:

	unsigned int tag_;

	unsigned int num_;

	std::string message_;

	enum response response_;
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

}
