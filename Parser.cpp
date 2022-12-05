#include "Parser.h"
#include "JsonListener.h"

void Listener::whitespace(char c)
{
}

void Listener::startDocument()
{
}

void Listener::key(String key)
{
  this->_key = key;
  keyFound = true;
}

void Listener::value(String value)
{
  this->_value = value;
  valueFound = true;
}

void Listener::endArray()
{
}

void Listener::endObject()
{
}

void Listener::endDocument()
{
}

void Listener::startArray()
{
}

void Listener::startObject()
{
}
