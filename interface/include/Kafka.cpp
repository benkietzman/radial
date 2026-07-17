// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Kafka.cpp
// author     : Ben Kietzman
// begin      : 2026-07-16
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Kafka"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Kafka()
Kafka::Kafka(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "kafka", argc, argv, pCallback)
{
  map<string, list<string> > watches;

  m_bLoad = false;
  // {{{ functions
  m_functions["reset"] = &Kafka::reset;
  m_functions["status"] = &Kafka::status;
  m_functions["topics"] = &Kafka::topics;
  // }}}
  load(strPrefix, true);
  watches[m_strData] = {".cred"};
  m_pThreadInotify = new thread(&Kafka::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
  m_pThreadSchedule = new thread(&Kafka::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Kafka()
Kafka::~Kafka()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
  m_pThreadInotify->join();
  delete m_pThreadInotify;
  for (auto &i : m_topics)
  {
    i.second->bExit = true;
    i.second->pThread->join();
    delete i.second->pThread;
  }
}
// }}}
// {{{ autoMode()
void Kafka::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Kafka::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
    m_bLoad = true;
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Kafka::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Kafka::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (isMasterSettled())
  {
    if (isMaster())
    {
      if (!empty(ptJson, "Function"))
      {
        string strFunction = ptJson->m["Function"]->v;
        radialUser d;
        userInit(ptJson, d);
        if (m_functions.find(strFunction) != m_functions.end())
        {
          d.p->m["i"]->insert("_function", strFunction);
          if ((this->*m_functions[strFunction])(d, strError))
          {
            bResult = true;
          }
        }
        else
        {
          strError = "Please provide a valid Function.";
        }
        if (bResult)
        {
          if (exist(ptJson, "Response"))
          {
            delete ptJson->m["Response"];
          }
          ptJson->m["Response"] = d.p->m["o"];
          d.p->m.erase("o");
        }
        userDeinit(d);
      }
      else
      {
        strError = "Please provide the Function.";
      }
    }
    else
    {
      Json *ptLink = new Json(ptJson);
      ptLink->i("Interface", "kafka");
      ptLink->i("Node", master());
      if (hub("link", ptLink, strError))
      {
        bResult = true;
        if (exist(ptLink, "Response"))
        {
          ptJson->i("Response", ptLink->m["Response"]);
        }
      }
      delete ptLink;
    }
  }
  else
  {
    strError = "Master not known.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
}
// }}}
// {{{ callbackInotify()
void Kafka::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Kafka::callbackInotify()";
  if (strPath == m_strData && strFile == ".cred")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ consumer()
void Kafka::consumer(string strPrefix, const string strTopic, map<string, string> &config, bool &bExit)
{
  bool bLoad = false, bValidated = true;
  char szError[512] = "\0";
  rd_kafka_conf_t *ptConf = rd_kafka_conf_new();
  rd_kafka_t *ptConsumer;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Kafka::consumer()";
  for (auto &i : config)
  {
    if (rd_kafka_conf_set(ptConf, i.first.c_str(), i.second.c_str(), szError, 512))
    {
      bValidated = false;
      ssMessage.str("");
      ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " " << char(3) << "00,14 " << i.first << " " << char(3) << " " << char(2) << char(3) << "07rd_kafka_conf_set() " << szError << char(3) << char(2);
      chat("#kafka", ssMessage.str());
      ssMessage.str("");
      ssMessage << strPrefix << "->rd_kafka_conf_set() error [" << strTopic << "," << i.first << "]:  " << szError;
      log(ssMessage.str());
    }
  }
  if (bValidated)
  {
    if ((ptConsumer = rd_kafka_new(RD_KAFKA_CONSUMER, ptConf, szError, 512)) != NULL)
    {
      bool bSubscribed = false;
      rd_kafka_resp_err_t tError;
      rd_kafka_topic_partition_list_t *ptSubscription = rd_kafka_topic_partition_list_new(1);
      rd_kafka_poll_set_consumer(ptConsumer);
      ptConf = NULL;
      rd_kafka_topic_partition_list_add(ptSubscription, strTopic.c_str(), RD_KAFKA_PARTITION_UA);
      if ((tError = rd_kafka_subscribe(ptConsumer, ptSubscription)) == RD_KAFKA_RESP_ERR_NO_ERROR)
      {
        bSubscribed = true;
      }
      else
      {
        cerr << "rd_kafka_subscribe() " << rd_kafka_err2str(tError) << endl;
        bLoad = true;
        ssMessage.str("");
        ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " " << char(2) << char(3) << "07rd_kafka_subscribe() " << szError << char(3) << char(2);
        chat("#kafka", ssMessage.str());
        ssMessage.str("");
        ssMessage << strPrefix << "->rd_kafka_subscribe() error [" << strTopic << "]:  " << szError;
        log(ssMessage.str());
      }
      rd_kafka_topic_partition_list_destroy(ptSubscription);
      if (bSubscribed)
      {
        ssMessage.str("");
        ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " Subscription established.";
        chat("#kafka", ssMessage.str());
        ssMessage.str("");
        ssMessage << strPrefix << "->rd_kafka_subscribe() [" << strTopic << "]:  Subscription established.";
        log(ssMessage.str());
        while (!bExit)
        {
          rd_kafka_message_t *ptMessage;
          if ((ptMessage = rd_kafka_consumer_poll(ptConsumer, 250)) != NULL)
          {
            if (ptMessage->err == RD_KAFKA_RESP_ERR_NO_ERROR)
            {
              size_t unPosition[2];
              string strKey((char *)ptMessage->key, (size_t)ptMessage->key_len), strPayload((char *)ptMessage->payload, (size_t)ptMessage->len);
              if ((unPosition[0] = strPayload.find("\"eventCorrelationId\":\"")) != string::npos && strPayload.size() > (unPosition[0] + 22) && (unPosition[1] = strPayload.find("\"", (unPosition[0] + 22))) != string::npos)
              {
                string strID = strPayload.substr((unPosition[0] + 22), (unPosition[1] - (unPosition[0] + 22))), strSubPrefix, strTarget, strType;
                stringstream ssID(strID);
                if (getline(ssID, strSubPrefix, '|') && strSubPrefix == "radial" && getline(ssID, strType, '|') && (strType == "interface" || strType == "logger") && getline(ssID, strTarget, '|') && !strTarget.empty())
                {
                  if (strType == "interface")
                  {
                    kafkaMessage(strTarget, strPayload);
                  }
                  else
                  {
                    map<string, string> label = {{"ID", strID}, {"Interface", "kafka"}, {"Key", strKey}, {"Source", "Radial"}, {"Topic", strTopic}};
                    logger(strTarget, "message", label, strPayload);
                  }
                }
              }
            }
            else if (ptMessage->err != RD_KAFKA_RESP_ERR__PARTITION_EOF)
            {
              bExit = bLoad = true;
              cerr << "rd_kafka_consumer_poll() " << rd_kafka_message_errstr(ptMessage) << endl;
              bLoad = true;
              ssMessage.str("");
              ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " " << char(2) << char(3) << "07rd_kafka_consumer_poll() " << rd_kafka_message_errstr(ptMessage) << char(3) << char(2);
              chat("#kafka", ssMessage.str());
              ssMessage.str("");
              ssMessage << strPrefix << "->rd_kafka_consumer_poll() error [" << strTopic << "]:  " << rd_kafka_message_errstr(ptMessage);
              log(ssMessage.str());
            }
            rd_kafka_message_destroy(ptMessage);
          }
        }
        rd_kafka_consumer_close(ptConsumer);
        ssMessage.str("");
        ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " Subscription closed.";
        chat("#kafka", ssMessage.str());
        ssMessage.str("");
        ssMessage << strPrefix << "->rd_kafka_consumer_close() [" << strTopic << "]:  Subscription closed.";
        log(ssMessage.str());
      }
      rd_kafka_destroy(ptConsumer);
    }
    else
    {
      bLoad = true;
      ssMessage.str("");
      ssMessage << char(3) << "13,06 " << strTopic << " " << char(3) << " " << char(2) << char(3) << "07rd_kafka_new() " << szError << char(3) << char(2);
      chat("#kafka", ssMessage.str());
      ssMessage.str("");
      ssMessage << strPrefix << "->rd_kafka_new() error [" << strTopic << "]:  " << szError;
      log(ssMessage.str());
    }
  }
  else
  {
    bLoad = true;
  }
  if (bLoad)
  {
    bExit = m_bLoad = true;
  }
  threadDecrement();
}
// }}}
// {{{ load()
void Kafka::load(string strPrefix, const bool bSilent)
{
  string strError;
  stringstream ssMessage;
  Json *ptTopics = new Json;

  strPrefix += "->Kafka::load()";
  if (isMasterSettled() && isMaster())
  {
    if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"kafka"}, ptTopics, strError))
    {
      list<string> removals;
      m_mutex.lock();
      for (auto &topic : ptTopics->m)
      {
        bool bFirst = true;
        string strKey;
        stringstream ssKey;
        for (auto &config : topic.second->m)
        {
          if (bFirst)
          {
            bFirst = false;
          }
          else
          {
            ssKey << ",";
          }
          ssKey << config.first << "=" << config.second->v;
        }
        strKey = ssKey.str();
        if (m_topics.find(topic.first) == m_topics.end() || m_topics[topic.first]->bExit || m_topics[topic.first]->strKey != strKey)
        {
          radialKafkaTopic *ptTopic = new radialKafkaTopic;
          ptTopic->bExit = false;
          topic.second->flatten(ptTopic->config, true, false);
          ptTopic->strKey = strKey;
          if (m_topics.find(topic.first) != m_topics.end() && (m_topics[topic.first]->bExit || m_topics[topic.first]->strKey != strKey))
          {
            m_topics[topic.first]->bExit = true;
            m_topics[topic.first]->pThread->join();
            delete m_topics[topic.first];
          }
          ptTopic->pThread = new thread(&Kafka::consumer, this, strPrefix, topic.first, ref(ptTopic->config), ref(ptTopic->bExit));
          pthread_setname_np(ptTopic->pThread->native_handle(), "consumer");
          m_topics[topic.first] = ptTopic;
        }
      }
      for (auto &topic : m_topics)
      {
        if (ptTopics->m.find(topic.first) == ptTopics->m.end())
        {
          removals.push_back(topic.first);
        }
      }
      while (!removals.empty())
      {
        m_topics[removals.front()]->bExit = true;
        m_topics[removals.front()]->pThread->join();
        delete m_topics[removals.front()];
        m_topics.erase(removals.front());
      }
      m_mutex.unlock();
    }
    else if (!bSilent)
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->Warden::vaultRetrieve() error [kafka]:  " << strError;
      log(ssMessage.str());
    }
    delete ptTopics;
  }
  else
  {
    m_mutex.lock();
    for (auto &topic : m_topics)
    {
      topic.second->bExit = true;
      topic.second->pThread->join();
      delete topic.second->pThread;
    }
    m_topics.clear();
    m_mutex.unlock();
  }
}
// }}}
// {{{ reset()
bool Kafka::reset(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (isLocalAdmin(d, "Radial"))
  {
    if (dep({"Topic"}, i, e))
    {
      m_mutex.lock();
      if (m_topics.find(i->m["Topic"]->v) != m_topics.end())
      {
        stringstream ssMessage;
        b = true;
        ssMessage << char(3) << "13,06 " << i->m["Topic"]->v << " " << char(3) << " Subscription has been reset by " << d.u << ".";
        chat("#kafka", ssMessage.str());
        ssMessage.str("");
        ssMessage << "Terminal::reset() [" << i->m["Topic"]->v << "]:  Subscription has been reset by " << d.u << ".";
        log(ssMessage.str());
        m_topics[i->m["Topic"]->v]->bExit = true;
      }
      else
      {
        e = "Please provide a valid Topic.";
      }
      m_mutex.unlock();
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ schedule()
void Kafka::schedule(string strPrefix)
{ 
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  time_t CTime[2] = {0, 0};
    
  threadIncrement();
  strPrefix += "->Kafka::schedule()";
  time(&(CTime[1]));
  // }}}
  while (!shutdown())
  {
    time(&(CTime[0]));
    if ((CTime[0] - CTime[1]) >= 10)
    {
      CTime[1] = CTime[0];
      if (m_bLoad)
      {
        load(strPrefix);
      }
    }
    msleep(1000);
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
// {{{ topics()
bool Kafka::topics(radialUser &d, string &e)
{
  bool b = true;
  Json *o = d.p->m["o"];

  m_mutex.lock();
  for (auto &topic : m_topics)
  {
    o->pb(topic.first);
  }
  m_mutex.unlock();

  return b;
}
// }}}
}
}
