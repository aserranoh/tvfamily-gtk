
#ifndef REQUESTSEARCHTHREAD_H
#define REQUESTSEARCHTHREAD_H

#include "searchlistener.h"

class SearchRequestThread {

    public:

        SearchRequestThread(
            const std::string& category,
            const std::string& search,
            const SearchListener& listener);

        ~SearchRequestThread();

        void operator()();

};

#endif

