#ifndef EZM_MOCK_UTILS_H
#define EZM_MOCK_UTILS_H

#ifdef TEST

#define EZM_MOCK_UT_CLEAR_LINE() \
    printf("\033[1A"); \
    printf("\033[2K")
#define EZM_MOCK_UT_LOG(msg, ...) \
    printf(msg "\n", ##__VA_ARGS__)

#define STATIC_GLOBAL 


#else

#define STATIC_GLOBAL static
#define EZM_MOCK_UT_CLEAR_LINE()
#define EZM_MOCK_UT_LOG(msg, ...)

#endif

#endif // EZM_MOCK_UTILS_H