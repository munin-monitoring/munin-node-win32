
#pragma once

class JCSocket
{
public:
  enum ProtocolType {
    TCP,
    UDP
  };

  JCSocket();
  virtual ~JCSocket();

  virtual bool Create(int af = AF_INET, ProtocolType = TCP);
  virtual bool Connect(const char *host, int nPort);
  virtual bool Bind(int nLocalPort, const char *address = NULL);
  virtual bool Accept(JCSocket *pSocket);
  virtual bool Listen(int nBacklog = SOMAXCONN);
  virtual int Send(const void *pData, int nDataLen, int nFlags = 0);
  virtual int SendText(const char *pszText);
  virtual int Recv(void *pData, int nDataLen, int nFlags = 0);
  virtual int RecvLine(char *pszBuf, int nLen, bool bEcho = false);
  virtual int RecvFrom(void *pData, int nDataLen, int nFlags = 0);
  virtual bool Shutdown(int nHow);
  virtual bool Close();

  SOCKET m_hSocket;
  SOCKADDR_IN m_Address;
  SOCKADDR_IN m_FromAddress;
private:
  static int m_RefCount;
};
