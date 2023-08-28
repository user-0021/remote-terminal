#pragma once

#include <stdint.h>
#include <arpa/inet.h>

typedef void* UDP_SERVER_HANDLE;
typedef void* UDP_CLIENT_HANDLE;
typedef void(*UDP_CALLBACK)(UDP_SERVER_HANDLE this,void* buff,size_t size,struct sockaddr_in from,void* userData);

typedef enum 
{
  UDP_HANDLE_SUCCESS,
  UDP_HANDLE_FAILED_SOCKET,
  UDP_HANDLE_FAILED_BIND
} UDP_HANDLE_STATUS;


/**
 * @brief udp server create
 * 
 * @param [in,out] handle created udp server obj 
 * @param [in] ip udp server listen ip 
 * @param [in] port udp server listen port
 * @param [in] callback udp callback when recived
 * @param [in]     userData userData in callback arg 
 * @return UDP_HANDLE_STATUS
 */
UDP_HANDLE_STATUS UdpServer_Create(UDP_SERVER_HANDLE* handle,in_addr_t ip,uint16_t port,UDP_CALLBACK callback,void* userData);

/**
 * @brief udp server loop
 * @details when cought udp message call callback
 * 
 * @param [in] handle udp object handle 
 */
void UdpServer_Loop(UDP_SERVER_HANDLE handle);

/**
 * @brief udp server send to client
 * 
 * @param [in] handle 
 * @param [in] addr client address 
 * @param [in] buff data buff
 * @param [in] size data buff size
 * 
 * @return ssize_t same sendto
 */
ssize_t UdpServer_Send(UDP_SERVER_HANDLE handle,struct sockaddr_in addr,void* buff,size_t size);

/**
 * @brief Close udp server
 * 
 * @param [in] handle 
 */
void CloseUdpServer(UDP_SERVER_HANDLE handle);

/**
 * @brief udp client create
 * 
 * @param [in,out] handle created udp client obj
 * @param [in,out] ip connect server ip
 * @param [in,out] port connect server port
 * @param [in,out] callback udp callback when recv
 * @param [in]     userData userData in callback arg 
 * @return UDP_HANDLE_STATUS 
 */
UDP_HANDLE_STATUS UdpClient_Create(UDP_CLIENT_HANDLE* handle,in_addr_t ip,uint16_t port,UDP_CALLBACK callback,void* userData);

/**
 * @brief udp client loop
 * @details when cought udp message call callback
 * 
 * @param [in] handle 
 */
void UdpClient_Loop(UDP_CLIENT_HANDLE handle);

/**
 * @brief udp client send to server
 * 
 * @param [in] handle 
 * @param [in] addr client address 
 * @param [in] buff data buff
 * @param [in] size data buff size
 * 
 * @return ssize_t same sendto
 */
ssize_t UdpClient_Send(UDP_CLIENT_HANDLE handle,void* buff,size_t size);

/**
 * @brief close udp client
 * 
 * @param [in] handle 
 */
void CloseUdpClient(UDP_CLIENT_HANDLE handle);