#pragma once

#include "JsonListener.h"

class Listener: public JsonListener {

  public:
    String _key;
    String _value;
    bool keyFound;
    bool valueFound;

    virtual void whitespace(char c);
  
    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};
