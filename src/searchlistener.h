
#ifndef SEARCHLISTENER_H
#define SEARCHLISTENER_H

#include "searchresult.h"

class SearchListener {

    public:

        virtual void search_finished (SearchResult& r) = 0;

};

#endif

