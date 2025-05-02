// -*- C++ -*-
// Radial
// -------------------------------------
// file       : jwt.cpp
// author     : Ben Kietzman
// begin      : 2023-02-17
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Jwt"
using namespace radial;
Jwt *gpJwt = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "jwt->main()";
  gpJwt = new Jwt(strPrefix, argc, argv, &callback);
  gpJwt->enableWorkers();
  gpJwt->process(strPrefix);
  delete gpJwt;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpJwt->callback(strPrefix, strPacket, bResponse);
}
