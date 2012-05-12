#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include <stdio.h>
#include "Acceptor.h"

struct acceptor
{
	on_accept accept_callback;
	SOCKET    sock;
	FD_SET Set;
	void *ud;
};

acceptor_t create_acceptor(const char *ip,unsigned long port,on_accept accept_callback,void *ud)
{
	SOCKET ListenSocket;
	struct sockaddr_in    addr;
	int                         optval=1;                        //Socket����ֵ
	unsigned long               ul=1;
	acceptor_t a;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == ListenSocket) 
	{
		printf("\nError occurred while opening socket: %d.", WSAGetLastError());
		return NULL;
	}

	addr.sin_family		= AF_INET;
	addr.sin_port		= htons((u_short)port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	if ((bind(ListenSocket, (struct sockaddr *)&addr, sizeof( struct sockaddr_in))) == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		return NULL;
	}

	if((listen(ListenSocket, 5)) == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		return NULL;
	}
	ioctlsocket(ListenSocket,FIONBIO,(unsigned long*)&ul);

	a = malloc(sizeof(*a));
	a->sock = ListenSocket;
	a->accept_callback = accept_callback;
	a->ud = ud;
	FD_ZERO(&a->Set);
	FD_SET(a->sock,&a->Set);
	return a;
}

void destroy_acceptor(acceptor_t *a)
{
	closesocket((*a)->sock);
	free(*a);
	*a = NULL;
}

void acceptor_run(acceptor_t a,int ms)
{
	struct timeval timeout;
	SOCKET client;
	struct sockaddr_in ClientAddress;
	int nClientLength = sizeof(ClientAddress);
	DWORD tick,_timeout,_ms;
	tick = GetTickCount();
	_timeout = tick + ms;
	do
	{
		_ms = _timeout - tick;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000*_ms;
		if(select(0, &a->Set,NULL, NULL, &timeout) >0 )
		{
			for(;;)
			{
				client = accept(a->sock, (struct sockaddr*)&ClientAddress, &nClientLength);
				if (INVALID_SOCKET == client)
					break;
				else
				{
					a->accept_callback(client,a->ud);
				}
			}
		}

		FD_ZERO(&a->Set);
		FD_SET(a->sock,&a->Set);
		tick = GetTickCount();
	}while(tick < _timeout);

}