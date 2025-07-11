#define ENSURE(condition) \
    if (!(condition)) \
    { \
        std::cerr << "ensure failed!" \
            << "\ncondition = "#condition \
            << "\n     file = " << __FILE__ \
            << "\n     line = " << __LINE__ \
            << std::endl; \
        throw std::logic_error("condiion is false: "#condition); \
    }

