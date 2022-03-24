#if !defined(_SRC_WHEEL_HPP_)
#define _SRC_WHEEL_HPP_

#include <map>
#include <string>

namespace crosswrench {

class wheel
{
  public:
    wheel() = delete;
    wheel(std::string);
    bool root_is_purelib();
    unsigned long version_major();
    unsigned long version_minor();
    bool wheel_version_unsupported();

  private:
    bool RootIsPurelib;
    unsigned long WheelVersionMajor;
    unsigned long WheelVersionMinor;
};

} // namespace crosswrench
#endif
