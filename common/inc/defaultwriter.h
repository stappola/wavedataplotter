#include "dataoutputif.h"


class DefaultWriter : public OutputWriterIF {
    public:
        DefaultWriter();
        ~DefaultWriter();
    public:   // API from OutputWriterIF
        virtual void writeToStream(const std::string& data) override;
};

