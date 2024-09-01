#include <string>


class OutputWriterIF {
    public:   // Virtual destructor
        virtual ~OutputWriterIF() {}
    public:   // API
        virtual void writeToStream(const std::string& data) = 0;
};

