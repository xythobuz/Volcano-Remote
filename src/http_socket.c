/*
 * http_socket.c
 *
 * Based on BittyHTTP example socket interface.
 *
 * Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "SocketsCon.h"

#include "config.h"
#include "log.h"
#include "main.h"
#include "ring.h"
#include "http.h"

#define MAX_SOCK 4
#define SOCK_RECV_BUFF 512

#define HTTP_DNS_TIMEOUT_MS 5000
#define HTTP_CONNECT_TIMEOUT_MS 5000

struct tcp_sock {
    bool set;
    struct tcp_pcb *pcb;

    // TODO listening server socket has unused buffer
    uint8_t rx_buf[SOCK_RECV_BUFF];
    struct ring_buffer rx_rb;

    struct tcp_pcb *child_buf[MAX_SOCK];
    struct ring_buffer child_rb;
};

static struct tcp_sock sock[MAX_SOCK] = {0};

bool SocketsCon_InitSocketConSystem(void) {
    return true;
}

void SocketsCon_ShutdownSocketConSystem(void) { }

static void tcp_server_err(void *arg, err_t err) {
    debug("tcp error %d", err);

    struct SocketCon *Con = arg;
    Con->ErrorCode = e_ConnectErrorMAX;
    Con->State = e_ConnectState_Error;
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    (void)tpcb;
    debug("tcp recv %d %d", p->len, err);

    struct SocketCon *Con = arg;
    size_t idx = Con->SocketFD;

    if (rb_space(&sock[idx].rx_rb) < p->len) {
        debug("not enough space (%d < %d)", rb_space(&sock[idx].rx_rb), p->len);
        tcp_abort(sock[idx].pcb);
        Con->ErrorCode = e_ConnectErrorMAX;
        Con->State = e_ConnectState_Error;
        return ERR_ABRT;
    }

    rb_add(&sock[idx].rx_rb, p->payload, p->len);
    return ERR_OK;
}

bool SocketsCon_InitSockCon(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return false;
    }

    ssize_t next = -1;
    for (size_t i = 0; i < MAX_SOCK; i++) {
        if (!sock[i].set) {
            next = i;
            break;
        }
    }

    if (next < 0) {
        debug("error: too many sockets");
        return false;
    }

    debug("new socket at %d", next);

    sock[next].pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (sock[next].pcb == NULL) {
        debug("error allocating new socket");
        return false;
    }

    tcp_arg(sock[next].pcb, Con);
    tcp_err(sock[next].pcb, tcp_server_err);
    tcp_recv(sock[next].pcb, tcp_server_recv);

    Con->SocketFD = next;
    sock[next].set = true;

    struct ring_buffer tmp = RB_INIT(sock[next].rx_buf, SOCK_RECV_BUFF, sizeof(uint8_t));
    sock[next].rx_rb = tmp;

    struct ring_buffer tmp2 = RB_INIT(sock[next].child_buf, MAX_SOCK, sizeof(struct tcp_pcb *));
    sock[next].child_rb = tmp2;

    Con->ErrorCode = e_ConnectError_AllOk;
    Con->State = e_ConnectState_Idle;
    Con->ReadInProgress = false;

    return true;
}

static err_t tcp_server_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    (void)tpcb;
    debug("tcp connected");

    struct SocketCon *Con = arg;
    Con->State = (err == ERR_OK) ? e_ConnectState_Connected : e_ConnectState_Idle;
    return ERR_OK;
}

bool SocketsCon_Connect(struct SocketCon *Con,
                        const char *ServerName, int portNo) {
    if ((!Con) || (!ServerName)) {
        debug("invalid param");
        return false;
    }

    ip_addr_t ipaddr;
    err_t err;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    do {
        err = dns_gethostbyname(ServerName, &ipaddr, NULL, NULL);
        main_loop_hw();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_time) >= HTTP_DNS_TIMEOUT_MS) {
            break;
        }
    } while (err == ERR_INPROGRESS);
    if (err != ERR_OK) {
        debug("error getting IP for '%s'", ServerName);
        return false;
    } else {
        debug("IP %s for '%s'", ip4addr_ntoa(&ipaddr), ServerName);
    }

    Con->State = e_ConnectState_Connecting;

    size_t idx = Con->SocketFD;
    err = tcp_connect(sock[idx].pcb,
                            &ipaddr, portNo,
                            tcp_server_connected);
    if (err != ERR_OK) {
        debug("error connecting (%d)", err);
        Con->State = e_ConnectState_Idle;
        return false;
    }

    start_time = to_ms_since_boot(get_absolute_time());
    while (Con->State == e_ConnectState_Connecting) {
        main_loop_hw();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_time) >= HTTP_CONNECT_TIMEOUT_MS) {
            break;
        }
    }

    // TODO ?
    //if (Con->State == e_ConnectState_Error) {
    //    Con->State = e_ConnectState_Idle;
    //}

    return (Con->State == e_ConnectState_Connected);
}

bool SocketsCon_EnableAddressReuse(struct SocketCon *Con, bool Enable) {
    (void)Con;
    (void)Enable;
    return true;
}

void SocketsCon_Tick(struct SocketCon *Con) { (void)Con; }

bool SocketsCon_Write(struct SocketCon *Con, const void *buf, int num) {
    if ((!Con) || (!buf) || (num <= 0)) {
        debug("invalid param");
        return false;
    }

    size_t idx = Con->SocketFD;
    err_t err = tcp_write(sock[idx].pcb,
                          buf, num, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        debug("error writing to socket");
        return false;
    }

    return true;
}

int SocketsCon_Read(struct SocketCon *Con, void *buf, int num) {
    if ((!Con) || (!buf) || (num <= 0)) {
        debug("invalid param");
        return false;
    }

    // TODO irq disable?

    size_t idx = Con->SocketFD;
    size_t len = rb_get(&sock[idx].rx_rb, buf, num);

    // TODO irq enable?

    return len;
}

void SocketsCon_Close(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return;
    }

    size_t idx = Con->SocketFD;
    sock[idx].set = false;

    err_t err = tcp_close(sock[idx].pcb);
    if (err != ERR_OK) {
        debug("error closing socket (%d)", err);
        // TODO retry?
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK) {
        debug("ignoring failed accept");
        return ERR_OK;
    }

    struct SocketCon *Con = arg;
    size_t idx = Con->SocketFD;
    if (rb_space(&sock[idx].child_rb) <= 0) {
        debug("no space for new connection");
        tcp_abort(newpcb);
        return ERR_OK; // ERR_ABRT ?
    }

    debug("new connection (%d)", err);

    rb_push(&sock[idx].child_rb, &newpcb);
    return ERR_OK;
}

bool SocketsCon_Listen(struct SocketCon *Con, const char *bindadd, int PortNo) {
    if (!Con) {
        debug("invalid param");
        return false;
    }

    ip_addr_t ipaddr;
    err_t err;
    if (bindadd) {
        uint32_t start_time = to_ms_since_boot(get_absolute_time());
        do {
            err = dns_gethostbyname(bindadd, &ipaddr, NULL, NULL);
            main_loop_hw();

            uint32_t now = to_ms_since_boot(get_absolute_time());
            if ((now - start_time) >= HTTP_DNS_TIMEOUT_MS) {
                break;
            }
        } while (err == ERR_INPROGRESS);
        if (err != ERR_OK) {
            debug("error getting IP for '%s'", bindadd);
            return false;
        } else {
            debug("IP %s for '%s'", ip4addr_ntoa(&ipaddr), bindadd);
        }
    }

    size_t idx = Con->SocketFD;
    err = tcp_bind(sock[idx].pcb,
                   bindadd ? &ipaddr : IP_ANY_TYPE,
                   PortNo);
    if (err != ERR_OK) {
        debug("error binding to '%s'", bindadd);
        return false;
    }

    struct tcp_pcb *tmp = tcp_listen(sock[idx].pcb);
    if (tmp == NULL) {
        debug("error listening on socket");
        return false;
    }
    sock[idx].pcb = tmp;

    tcp_accept(sock[idx].pcb, tcp_server_accept);

    return true;
}

bool SocketsCon_Accept(struct SocketCon *Con, struct SocketCon *NewCon) {
    if ((!Con) || (!NewCon)) {
        debug("invalid param");
        return false;
    }

    size_t idx = Con->SocketFD;
    size_t new_idx = NewCon->SocketFD;

    if (rb_len(&sock[idx].child_rb) <= 0) {
        return false;
    }

    debug("accepting new connection");

    struct tcp_pcb *new_pcb;
    rb_pop(&sock[idx].child_rb, &new_pcb);

    err_t err = tcp_close(sock[new_idx].pcb);
    if (err != ERR_OK) {
        debug("error closing prev new socket");
    }

    sock[new_idx].pcb = new_pcb;
    return true;
}

bool SocketsCon_HasError(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return true;
    }
    return Con->State == e_ConnectState_Error;
}

bool SocketsCon_IsConnected(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return false;
    }
    return Con->State == e_ConnectState_Connected;
}

int SocketsCon_GetLastErrNo(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return -1;
    }
    return Con->Last_errno;
}

e_ConnectErrorType SocketsCon_GetErrorCode(struct SocketCon *Con) {
    if (!Con) {
        debug("invalid param");
        return e_ConnectErrorMAX;
    }
    return Con->ErrorCode;
}

bool SocketsCon_GetSocketHandle(struct SocketCon *Con,
                                t_ConSocketHandle *RetHandle) {
    if (!Con) {
        debug("invalid param");
        return false;
    }

    if (Con->SocketFD < 0) {
        return false;
    }

    *RetHandle = Con->SocketFD;
    return true;
}
