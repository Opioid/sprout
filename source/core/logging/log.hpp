#ifndef SU_CORE_LOGGING_LOG_HPP
#define SU_CORE_LOGGING_LOG_HPP

#include <string>

namespace logging {

class Log {
  public:
    enum class Type { Info, Warning, Error, Verbose };

    virtual ~Log() noexcept;

    void post(Type type, std::string const& text = "") noexcept;

    void post(Type type, std::string const& text, float a) noexcept;

    void post(Type type, std::string const& text, std::string const& a) noexcept;

    void push(Type type, std::string const& text) noexcept;

    void push(Type type, std::string const& text, std::string const& a) noexcept;

  protected:
    virtual void internal_post(Type type, std::string const& text) noexcept = 0;

  private:
    struct Entry {
        Type type;

        std::string text;
    };

    static int32_t constexpr Num_entries = 1;

    int32_t current_entry_ = 0;

    Entry entries_[Num_entries];
};

}  // namespace logging

#endif
