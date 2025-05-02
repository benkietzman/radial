// -*- C++ -*-
// Radial
// -------------------------------------
// file       : request.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Request"
using namespace radial;
Request *gpRequest;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "request->main()";
  gpRequest = new Request(strPrefix, argc, argv, &callback);
  gpRequest->enableWorkers();
  thread threadAccept(&Request::accept, gpRequest, strPrefix);
  pthread_setname_np(threadAccept.native_handle(), "accept");
  gpRequest->process(strPrefix);
  threadAccept.join();
  delete gpRequest;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpRequest->callback(strPrefix, strPacket, bResponse);
}
