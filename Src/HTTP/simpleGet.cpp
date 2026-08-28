//**************************************************************************************************
//* FILE:		simpleGet.cpp
//*
//* DESCRIP:	
//*
//*




#include <winsock2.h>

#include "simpleGet.h"
#include "stringUtils.h"

#include <map>
#include <crtdbg.h>
#include <cstring>



using namespace std;
using namespace stringUtil;

static map<int,string> s_errMsgs;

class win32_socket_init
{
	bool m_bError;

public:
    ~win32_socket_init()
    {
        ::WSACleanup();
    }
    win32_socket_init() :
		m_bError(false)
    {

        WORD    wVersion = MAKEWORD( 1, 1 );
        WSADATA wsaData;

        int nErr = ::WSAStartup(wVersion, &wsaData);
		if (nErr != 0)
		{
			//* WSAStartup failed
			m_bError = true;
		}

        s_errMsgs[WSAEINTR]			= "Interrupted function call: A blocking operation was interrupted by a call to WSACancelBlockingCall.";
        s_errMsgs[WSAEFAULT]		= "Bad address: The system detected an invalid pointer address in attempting to use a pointer argument of a call.";
        s_errMsgs[WSAEINVAL]		= "Invalid argument: Some invalid argument was supplied.";
        s_errMsgs[WSAEMFILE]		= "Too many open descriptors: No more socket descriptors are available.";
        s_errMsgs[WSAEWOULDBLOCK]	= "Call would block: Non-blocking call will block.";
        s_errMsgs[WSAEINPROGRESS]	= "Operation now in progress: A blocking operation is currently executing.";
        s_errMsgs[WSAEALREADY]		= "Operation already in progress: An operation was attempted on a nonblocking socket with an operation already in progress.";
        s_errMsgs[WSAENOTSOCK]		= "Socket operation on non socket: An operation was attempted on something that is not a socket.";
        s_errMsgs[WSAEDESTADDRREQ]	= "Destination address required: A required address was omitted from an operation on a socket.";
        s_errMsgs[WSAEMSGSIZE]		= "Message too long: A message sent on a datagram socket was larger than the internal message buffer.";
        s_errMsgs[WSAEPROTOTYPE]	= "The specified protocol is the wrong type for this socket.";
        s_errMsgs[WSAENOPROTOOPT]	= "Bad Protocol option.";
        s_errMsgs[WSAEPROTONOSUPPORT]	= "The specified protocol is not supported.";
        s_errMsgs[WSAESOCKTNOSUPPORT]	= "The specified socket type is not supported in this address family.";
        s_errMsgs[WSAEOPNOTSUPP]		= "Socket operation not supported.";
        s_errMsgs[WSAEPFNOSUPPORT]	= "Protocol family not supported.";
        s_errMsgs[WSAEAFNOSUPPORT]	= "The specified address family is not supported";
        s_errMsgs[WSAEADDRINUSE]	= "Address already in use.";
        s_errMsgs[WSAEADDRNOTAVAIL]	= "Cannot assign requested address.";
        s_errMsgs[WSAENETDOWN]		= "A network subsystem or the associated service provider has failed";
        s_errMsgs[WSAENETUNREACH]	= "Nework is unreachable.";
        s_errMsgs[WSAENETRESET]		= "Network dropped connection on reset.";
        s_errMsgs[WSAECONNABORTED]	= "Software caused connection abort.";
        s_errMsgs[WSAECONNRESET]	= "Connection reset by peer.";
        s_errMsgs[WSAENOBUFS]		= "No buffer space is available. The socket cannot be created.";
        s_errMsgs[WSAEISCONN]		= "Socket is already connected.";
        s_errMsgs[WSAENOTCONN]		= "Socket is not connected.";
        s_errMsgs[WSAESHUTDOWN]		= "Cannot send after socket shutdown.";
        s_errMsgs[WSAETIMEDOUT]		= "Connection timed out.";
        s_errMsgs[WSAECONNREFUSED]	= "Connection refused.";
        s_errMsgs[WSAEHOSTDOWN]		= "Host is down.";
        s_errMsgs[WSAEHOSTUNREACH]	= "No route to host.";
        s_errMsgs[WSAEPROCLIM]		= "Too many processes.";
    }
};

static win32_socket_init win32_socket_init_force;

static string errMsg(int errCode) throw()
{
    auto i = s_errMsgs.find(errCode);
	if (i != s_errMsgs.end())
	{
		return (i == s_errMsgs.end() ? "error code " + tos(errCode) : (*i).second);
	}

	return "";
}


static void socketWrite(SOCKET s, const void *data, int len, DWORD msTimeout) throw(runtime_error)
{
    _ASSERTE(s != INVALID_SOCKET);
    if ((!data) || (len <= 0)) return;

    // assumes non-blocking socket
    const __int8 *dp = reinterpret_cast<const __int8 *>(data);

    DWORD startT = ::timeGetTime();

    while (len)
    {
        int err = ::send(s, dp, len, 0);
        if (err == SOCKET_ERROR)
        {
            int errCode = ::WSAGetLastError();
            if (errCode != WSAEWOULDBLOCK)
			{
                throw runtime_error("simpleGet - socketWrite() - socket error " + errMsg(errCode));
			}
            
			DWORD t = ::timeGetTime();
            if (t < startT)
			{
                startT = t;
			}

            if ((t - startT) > msTimeout)
			{
                throw runtime_error("simpleGet socketWrite() - socket timeout");
			}

            ::Sleep(5);
        }
        else if (err > len)
        {
            throw runtime_error("bizarro error in simpleGet - socketWrite(). Wrote more than possible");
        }
        else
        {
            len -= err;
            dp += err;
        }
    }
}
static void socketWrite(SOCKET s,const string &data,DWORD msTimeout) throw(runtime_error)
{
    socketWrite(s,data.c_str(),data.size(),msTimeout);
}

static string getLine(SOCKET s,DWORD timeoutInMS) throw(runtime_error)
{
    _ASSERTE(s != INVALID_SOCKET);
    // assumes non-blocking socket
    DWORD startT = ::timeGetTime();
    string result;
    char c(0);

    while (true)
    {
        int err = ::recv(s,&c,1,0);
        if (err == 1)
        {
            if (c == '\n')
                break;

            if (c != '\r')
                result += c;
        }
        else if (err == 0)
		{
            throw runtime_error("simpleGet - getLine() - socket closed");
		}
        else if (err == SOCKET_ERROR)
        {
            int errCode = ::WSAGetLastError();
            if (errCode != WSAEWOULDBLOCK)
			{
                throw runtime_error("simpleGet - getLine() - socket error " + errMsg(errCode));
			}

            DWORD t = ::timeGetTime();
            if (t < startT)
			{
                startT = t;
			}

            if ((t - startT) > timeoutInMS)
			{
                throw runtime_error("simpleGet - getLine() - socket timeout");
			}

            ::Sleep(5);
        }
        else
            throw runtime_error("simpleGet - getLine() - bizarro error");
    }

    return result;
}

static string getData(SOCKET s, DWORD timeoutInMS) throw(runtime_error)
{
    _ASSERTE(s != INVALID_SOCKET);
    // assumes non-blocking socket
    DWORD startT = ::timeGetTime();
    
	string result;
    static const int MAXREAD = 1024;
	char c[MAXREAD + 1];

    while (true)
    {
        int err = ::recv(s,c,MAXREAD,0);
        if (err > 0 && err <= MAXREAD)
        {
            c[err] = 0;
            result += c;
        }
        else if (err == 0)
		{
			::Sleep(5);
            break; // done
		}
        else if (err == SOCKET_ERROR)
        {
            int errCode = ::WSAGetLastError();
            if (errCode != WSAEWOULDBLOCK)
			{
                throw runtime_error("simpleGet - getData() - socket error " + errMsg(errCode));
			}

            DWORD t = ::timeGetTime();
            if (t < startT)
			{
                startT = t;
			}

            if ((t - startT) > timeoutInMS)
			{
                throw runtime_error("simpleGet - getData() - socket timeout");
			}

            ::Sleep(5);
        }
        else
		{
            throw runtime_error("simpleGet - getLine() - bizarro error");
		}
    }

    return result;
}

string simpleGet(const string &request_url) throw(exception)
{
    string result;
    int		port = 80;

    string url = request_url;

    if (url.find("http://") == string::npos)
        throw runtime_error("Not an http url");

    url = url.substr(7);
    string::size_type pos = url.find("/");
    if (pos == string::npos)
    {
        url += "/";
        pos = url.find("/");
    }
    string address = url.substr(0,pos);
    string path = url.substr(pos);
    pos = address.find(":");
    if (pos != string::npos)
    {
        port = atoi(address.substr(pos+1).c_str());
        address = address.substr(0,pos);
    }
    if (address == "")
        throw runtime_error("Address is blank");

    // ok, we've got the necessary components
    SOCKET s = INVALID_SOCKET;
    try
    {
        struct hostent    *ph = 0;
        struct sockaddr_in addr;

        if( (s = ::socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
            throw runtime_error("Error opening socket " + tos(::WSAGetLastError()));

        if(isdigit((int) (address[0])) && (address.find(".") != string::npos))
        {
            unsigned long iaddr = inet_addr(address.c_str());
            ph = ::gethostbyaddr( (const char *)&iaddr, 4, PF_INET );
        }
        else
        {
            ph = ::gethostbyname(address.c_str());
        }

        if( ph == NULL )
            throw runtime_error("Error resolving host gethostbyaddr or gethostbyname failed for " + address + " " + tos(::WSAGetLastError()));

        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        memcpy(&addr.sin_addr, ph->h_addr_list[0],sizeof(struct in_addr) );

        if( ::connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)) )
            throw runtime_error("Connect failed to " + address + ":" + tos(port) + " " + tos(::WSAGetLastError()));

        unsigned long nonblock = 1;
        if (::ioctlsocket(s,FIONBIO,&nonblock) == SOCKET_ERROR)
            throw runtime_error("Could not set socket to non blocking");

        // write request
        socketWrite(s,"GET " + path + " HTTP/1.0\r\nHost: " + address + "\r\n\r\n",60 * 1000);
        // get, but ignore, all headers
        while (true)
        {
            string header = getLine(s,60 * 1000);
            if (header == "")
                break;
        }
        // snarf data
        result = getData(s,5 * 60 * 1000);

        ::closesocket(s);
        s = INVALID_SOCKET;
    }
    catch(...)
    {
        if (s != INVALID_SOCKET)
            ::closesocket(s);
        s = INVALID_SOCKET;
        throw;
    }

    return result;
}
