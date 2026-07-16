// -*- C++ -*-
// Radial
// -------------------------------------
// file       : kafka.cpp
// author     : Ben Kietzman
// begin      : 2026-07-16
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Kafka"
using namespace radial;
Kafka *gpKafka;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "kafka->main()";
  gpKafka = new Kafka(strPrefix, argc, argv, &callback, &callbackInotify);
  gpKafka->setAutoMode(&autoMode);
  gpKafka->enableWorkers();
  gpKafka->process(strPrefix);
  delete gpKafka;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Kafka::autoMode, gpKafka, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpKafka->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpKafka->callbackInotify(strPrefix, strPath, strFile);
}
