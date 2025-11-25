#include "cpptoml.h"

#include <iostream>
#include <limits>

/**
 * A visitor for toml objects that writes to an output stream in the JSON
 * format that the toml-test suite expects.
 */
class toml_test_writer
{
  public:
    toml_test_writer(std::ostream& s) : stream_(s)
    {
        // nothing
    }

    void visit(const cpptoml::value<std::string>& v)
    {
        stream_ << "{\"type\":\"string\",\"value\":\""
                << cpptoml::toml_writer::escape_string(v.get()) << "\"}";
    }

    void visit(const cpptoml::value<int64_t>& v)
    {
        stream_ << "{\"type\":\"integer\",\"value\":\"" << v.get() << "\"}";
    }

    void visit(const cpptoml::value<double>& v)
    {
        stream_ << "{\"type\":\"float\",\"value\":\"" << v.get() << "\"}";
    }

    void visit(const cpptoml::value<cpptoml::local_date>& v)
    {
        stream_ << "{\"type\":\"local_date\",\"value\":\"" << v.get() << "\"}";
    }

    void visit(const cpptoml::value<cpptoml::local_time>& v)
    {
        stream_ << "{\"type\":\"local_time\",\"value\":\"" << v.get() << "\"}";
    }

    void visit(const cpptoml::value<cpptoml::local_datetime>& v)
    {
        stream_ << "{\"type\":\"local_datetime\",\"value\":\"" << v.get()
                << "\"}";
    }

    void visit(const cpptoml::value<cpptoml::offset_datetime>& v)
    {
        stream_ << "{\"type\":\"datetime\",\"value\":\"" << v.get() << "\"}";
    }

    void visit(const cpptoml::value<bool>& v)
    {
        stream_ << "{\"type\":\"bool\",\"value\":\"" << v << "\"}";
    }

    void visit(const cpptoml::array& arr)
    {
        stream_ << "{\"type\":\"array\",\"value\":[";
        auto it = arr.get().begin();
        while (it != arr.get().end())
        {
            (*it)->accept(*this);
            if (++it != arr.get().end())
                stream_ << ", ";
        }
        stream_ << "]}";
    }

    void visit(const cpptoml::table_array& tarr)
    {
        stream_ << "[";
        auto arr = tarr.get();
        auto ait = arr.begin();
        while (ait != arr.end())
        {
            (*ait)->accept(*this);
            if (++ait != arr.end())
                stream_ << ", ";
        }
        stream_ << "]";
    }

    void visit(const cpptoml::table& t)
    {
        stream_ << "{";
        auto it = t.begin();
        while (it != t.end())
        {
            stream_ << '"' << cpptoml::toml_writer::escape_string(it->first)
                    << "\":";
            it->second->accept(*this);
            if (++it != t.end())
                stream_ << ", ";
        }
        stream_ << "}";
    }

  private:
    std::ostream& stream_;
};

int main(int argc, char** argv)
{
    std::cout.precision(std::numeric_limits<double>::max_digits10);
    
    // Default timeout is 30 seconds
    std::chrono::milliseconds timeout = std::chrono::seconds(30);
    
    // Parse command line arguments
    if (argc == 2)
    {
        try
        {
            int timeout_seconds = std::stoi(argv[1]);
            if (timeout_seconds < 1)
            {
                std::cerr << "Timeout must be at least 1 second" << std::endl;
                return 1;
            }
            timeout = std::chrono::seconds(timeout_seconds);
        }
        catch (const std::invalid_argument&)
        {
            std::cerr << "Invalid timeout value: " << argv[1] << std::endl;
            return 1;
        }
        catch (const std::out_of_range&)
        {
            std::cerr << "Timeout value out of range: " << argv[1] << std::endl;
            return 1;
        }
    }
    else if (argc > 2)
    {
        std::cerr << "Usage: parse_stdin [timeout_seconds]" << std::endl;
        return 1;
    }
    
    cpptoml::parser p{std::cin, timeout};
    try
    {
        std::shared_ptr<cpptoml::table> g = p.parse();
        toml_test_writer writer{std::cout};
        g->accept(writer);
        std::cout << std::endl;
    }
    catch (const cpptoml::parse_exception& ex)
    {
        std::cerr << "Parsing failed: " << ex.what() << std::endl;
        return 1;
    }
    catch (const cpptoml::stream_read_timeout& ex)
    {
        std::cerr << "Reading from stdin timed out: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Something horrible happened!" << std::endl;
        // return as if there was success so that toml-test will complain
        return 0;
    }
    return 0;
}
