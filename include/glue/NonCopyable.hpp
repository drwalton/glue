#ifndef GLUE_NONCOPYABLE_HPP_INCLUDED
#define GLUE_NONCOPYABLE_HPP_INCLUDED

namespace glue {

class NonCopyable
{
public:
	NonCopyable() {}
private:
	NonCopyable(const NonCopyable &other);
	NonCopyable &operator=(const NonCopyable &other);
};

}

#endif
