/* This file is part of munin-node-win32
 * Copyright (C) 2006-2007 Jory Stone (jcsston@jory.info)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "StdAfx.h"
#include "JCSocket.h"

int JCSocket::m_RefCount = 0;

JCSocket::JCSocket()
{
  WSADATA wd;

  if (++m_RefCount == 1)
  {
    ::WSAStartup(0x0101, &wd);
  }
  memset(&m_Address, 0, sizeof(m_Address));
  memset(&m_FromAddress, 0, sizeof(m_FromAddress));
  m_hSocket = NULL;
}


JCSocket::~JCSocket()
{
  Close();

  if( --m_RefCount == 0 )
  {
    ::WSACleanup();
  }
}


bool JCSocket::Create( int af, ProtocolType type )
{
  switch (type) 
  {
    case TCP:
      m_hSocket = ::socket( af, SOCK_STREAM, IPPROTO_TCP );
      break;
    case UDP:
      m_hSocket = ::socket( af, SOCK_DGRAM, IPPROTO_UDP );
      break;
    default:
      assert(false);
  };

  if( m_hSocket == INVALID_SOCKET )
  {
    return false;
  }
  else
  {
    return true;
  }
}


bool JCSocket::Connect( const char *host, int nPort )
{
  unsigned long ulAddr = 0;
  hostent *pEnt = ::gethostbyname( host );
  SOCKADDR_IN addr;

  if( pEnt == 0 )
  {
    ulAddr = ::inet_addr( host );

    if( ulAddr == INADDR_NONE )
    {
#ifdef _DEBUG
      printf("Invalid address passed to JCSocket::Connect!");
#endif
      return false;
    }
    else
    {
      addr.sin_family = AF_INET;
    }
  }
  else
  {
    memcpy( &ulAddr, pEnt->h_addr_list[0], sizeof( long ) );

    addr.sin_family = pEnt->h_addrtype;
  }

  addr.sin_addr.s_addr = ulAddr;
  addr.sin_port = ::htons( nPort );

  memset( addr.sin_zero, 0, sizeof( addr.sin_zero ) );

  if( ::connect( m_hSocket, (const sockaddr *)&addr, sizeof( SOCKADDR_IN ) ) == SOCKET_ERROR )
  {
    return false;
  }
  else
  {
    return true;
  }
}


bool JCSocket::Bind(int nLocalPort, const char *address)
{
  m_Address.sin_addr.s_addr = address ? inet_addr(address) : INADDR_ANY;
  m_Address.sin_family = AF_INET;
  m_Address.sin_port = ::htons(nLocalPort);
  memset(m_Address.sin_zero, 0, sizeof(m_Address.sin_zero));

  if( ::bind( m_hSocket, (const sockaddr *)&m_Address, sizeof( SOCKADDR_IN ) ) == SOCKET_ERROR )
  {
    return false;
  }
  else
  {
    return true;
  }
}


bool JCSocket::Accept( JCSocket *pSocket )
{
  if( pSocket == NULL )
  {
    return false;
  }

  int len = sizeof( SOCKADDR_IN );
  memset( &pSocket->m_Address, 0, sizeof( SOCKADDR_IN ) );

  pSocket->m_hSocket = ::accept( m_hSocket, (sockaddr*)&pSocket->m_Address, &len );

  if( pSocket->m_hSocket == INVALID_SOCKET )
  {
    return false;
  }
  else
  {
    return true;
  }
}


bool JCSocket::Listen( int nBackLog )
{
  if( ::listen( m_hSocket, nBackLog ) == SOCKET_ERROR )
  {
    return false;
  }
  else
  {
    return true;
  }
}


int JCSocket::Send( const void *pData, int nDataLen, int nFlags )
{
  return ::send( m_hSocket, (const char *)pData, nDataLen, nFlags );
}


int JCSocket::SendText( const char *pszText )
{
  return Send( pszText, strlen( pszText ) );
}


int JCSocket::Recv( void *pData, int nDataLen, int nFlags )
{
  return ::recv( m_hSocket, (char *)pData, nDataLen, nFlags );
}


int JCSocket::RecvLine( char *pszBuf, int nLen, bool bEcho )
{
  int nCount = 0;
  int nRdLen;
  char ch = 0;

  while( ch != '\n' && nCount < nLen )
  {
    nRdLen = Recv( &ch, 1 );

    if( nRdLen == 0 || nRdLen == SOCKET_ERROR )
    {
      nCount = 0;
      break;
    }

    if( ch != '\n' && ch != '\r' )
    {
      pszBuf[nCount] = ch;
      nCount++;
    }

    if( bEcho )
    {
      Send( &ch, 1 );
    }
  }

  if( nCount != 0 )
  {
    pszBuf[nCount] = 0;
  }

  return nCount ? nCount : nRdLen;
}

int JCSocket::RecvFrom(void *pData, int nDataLen, int nFlags)
{
  int fromAddressLen = sizeof(m_FromAddress);
  return ::recvfrom( m_hSocket, (char *)pData, nDataLen, nFlags, (sockaddr *)&m_FromAddress, &fromAddressLen );
}

bool JCSocket::Shutdown( int nHow )
{
  return ::shutdown( m_hSocket, nHow ) == SOCKET_ERROR ? false : true;
}


bool JCSocket::Close( void )
{
  return ::closesocket( m_hSocket ) == SOCKET_ERROR ? false : true;
}
