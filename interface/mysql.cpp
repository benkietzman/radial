// -*- C++ -*-
// Radial
// -------------------------------------
// file       : mysql.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Mysql"
using namespace radial;
Mysql *gpMysql;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "mysql->main()";
  gpMysql = new Mysql(strPrefix, argc, argv, &callback);
  gpMysql->enableWorkers();
  gpMysql->process(strPrefix);
  delete gpMysql;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMysql->callback(strPrefix, strPacket, bResponse);
}
