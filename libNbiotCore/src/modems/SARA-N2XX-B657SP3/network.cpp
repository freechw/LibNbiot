/* ========================================================================
 * LibNbiot: nbiotconnectivity.cpp
 *
 * Copyright (c) 2018, Edgar Hindemith, Yassine Amraue, Thorsten
 * Krautscheid, Kolja Vornholt, T-Systems International GmbH
 * contact: libnbiot@t-systems.com, opensource@telekom.de
 *
 * This file is distributed under the conditions of the Apache License,
 * Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * For details see the file LICENSE at the toplevel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ========================================================================
*/

#include <stdlib.h>

#include "atcommands.h"
#include "nbiottimer.h"
#include "nbiotstringlist.h"
#include "nbiotdebug.h"

#include "network.h"

const char* Network::cmdNSOCR_arg = "AT+NSOCR=\"DGRAM\",17,%d\r";
const char* Network::respNSONMI_arg = "+NSONMI: %d,";
const char* Network::cmdNSORF_arg = "AT+NSORF=%d,%d\r";
const char* Network::cmdNSOST_arg = "AT+NSOST=%d,\"%s\",%d,%d,\"";
const char* Network::cmdNSOCL_arg = "AT+NSOCL=%d\r";

Network::Network(Serial& serial) :
    m_cmd(serial),
    m_connectionNumber(-1),
    m_hostname(),
    m_port(0),
    m_bytesAvail(0),
    m_nsonmi(),
    m_lastListenPort(listenPortBase-1)
{
    m_cmd.clearFilter();
}


bool Network::connect(const char* hostname, unsigned short port)
{
    bool ret = true;

    unsigned short listenPort = m_lastListenPort;
    if((0 <= m_connectionNumber) && (maxSocket > m_connectionNumber))
    {
        listenPort = (listenPortBase + m_connectionNumber);
    }

    if(m_cmd.sendCommand(nbiot::string::Printf(cmdNSOCR_arg, listenPort)))
    {
        if(!m_cmd.readResponse(REPLY_ANY, threeSeconds))
        {
            ret = false;
        }
    }
    else
    {
        ret = false;
    }
    if(ret)
    {
        nbiot::string response = m_cmd.getResponse();
        m_connectionNumber = atoi(response.c_str());
        if((0 <= m_connectionNumber) && (maxSocket > m_connectionNumber))
        {
            m_hostname = hostname;
            m_port = port;
            m_lastListenPort = listenPort;
        }
        else
        {
            m_hostname = "";
            m_port = 0;
            m_connectionNumber = -1;
            ret = false;
        }
    }
    m_cmd.readResponse(REPLY_IGNORE, oneSecond);
    if(ret)
    {
        m_nsonmi = nbiot::string::Printf(respNSONMI_arg, m_connectionNumber);
        m_cmd.addUrcFilter(m_nsonmi.c_str(), this, &Network::parseFilterResult);
    }
#ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
    debugPrintf("\033[0;32m[ MODEM    ]\033[0m ");
#endif
    debugPrintf("modem connect: %s\r\n", ((ret)?"ok":"fail"));
#endif
    return ret;
}

int Network::read(unsigned char* buffer, int len, unsigned short timeout_ms)
{
    int rc = -1;
    int dgmLen = 0;
    int reqLen = 0;
    int rb = -1;
    int bytes = 0;

    nbiot::Timer timer(timeout_ms);
    nbiot::string data;

    if (len >= 0)
    {
        reqLen = len;
    }
    else
    {
        return rc;
    }

    // Try to read requested length
    rb = ipRead(data, reqLen, readInterval);

    if(0 < rb)
    {
        bytes += rb;
    }

#ifdef DEBUG_MODEM
    bool dbgLine = false;
#endif

    // Loop and look for incoming messages until enough bytes available
    while (readInterval < timer.remaining() && bytes < reqLen)
    {
        m_cmd.readResponse(REPLY_IGNORE, readInterval);
        dgmLen = ipAvailable();

        // No bytes available
        if (dgmLen == 0)
        {

#ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
            if (!dbgLine)
            {
                debugPrintf("\033[0;32m[ MODEM    ]\033[0m ");
            }
#endif
            debugPrintf(".");
            dbgLine = true;
#endif
            continue;
        }

        // Enough bytes available
        if(reqLen <= (dgmLen + bytes))
        {
            break;
        }

        // Not enough bytes available but there is something we can read.
        if (0 < dgmLen)
        {
            rb = ipRead(data, dgmLen, readInterval);
            if(0 < rb)
            {
                bytes += rb;
            }
        }
    }

#ifdef DEBUG_MODEM
    if(dbgLine)
    {
        debugPrintf("\r\n");
    }
#endif

    // Read remaining bytes
    if(0 < dgmLen && bytes < reqLen)
    {
#ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
        debugPrintf("\033[0;32m[ MODEM    ]\033[0m ");
#endif
        debugPrintf("datagram-len: %d\r\n", dgmLen);
#endif
        unsigned short to = (readInterval < timer.remaining())?static_cast<unsigned short>(timer.remaining()):readInterval;
        rc = ipRead(data, reqLen - bytes, to) + bytes;
    }
    else
    {
        rc = bytes;
    }

#ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
    debugPrintf("\033[0;32m[ MODEM    ]\033[0m ");
#endif
    debugPrintf("r(%d): ", rc);
#endif

    if(0 < rc)
    {
        data.copy(buffer, static_cast<size_t>(rc));
#ifdef DEBUG_MODEM
        for(size_t i = 0; i < static_cast<size_t>(rc); ++i)
        {
            debugPrintf("%02X ", buffer[i]);
        }
#endif
    }
#ifdef DEBUG_MODEM
    debugPrintf("\r\n");
#endif
    return rc;
}

unsigned short Network::ipAvailable()
{
    return static_cast<unsigned short>((m_bytesAvail & byteCountMask));
}

int Network::ipRead(nbiot::string& data, int len, unsigned short timeout_ms)
{
    int avail = -1;

    nbiot::Timer timer(timeout_ms);
    if(m_cmd.sendCommand(nbiot::string::Printf(cmdNSORF_arg, m_connectionNumber, len)))
    {
        if(m_cmd.readResponse(REPLY_ANY, timeout_ms))
        {
            nbiot::string response = m_cmd.getResponse();
            if(0 == response.find("OK"))
            {
                avail = 0;
            }
            else
            {
                nbiot::StringList list = response.split(',');
                if(6 == list.count())
                {
                    if(m_connectionNumber == atoi(list[nsorfRespIdx_connNum].c_str()))
                    {
                        avail = atoi(list[nsorfRespIdx_bytesAvail].c_str());
                        if((0 < avail) && (avail <= len))
                        {
                            data.append(nbiot::string::fromHex(list[nsorfRespIdx_data].strip('"').c_str()));
                            m_bytesAvail = static_cast<size_t>(atoi(list[nsorfRespIdx_bytesRem].c_str()));
                        }
                    }
                }
            }
        }
    }

    m_cmd.readResponse(REPLY_IGNORE, static_cast<unsigned short>(timer.remaining()));

    return avail;
}

bool Network::write(unsigned char* buffer, unsigned long len, unsigned short timeout)
{
    bool ret = false;

    if((!m_hostname.empty()) && (0 < m_port))
    {
        nbiot::string data = nbiot::string::Printf(cmdNSOST_arg, m_connectionNumber, m_hostname.c_str(), m_port, len);
        nbiot::string hex = nbiot::string((char*)(buffer), len).toHex();
        data += hex;
        data += "\"";
#ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
        debugPrintf("\033[0;32m[ MODEM    ]\033[0m ");
#endif
        debugPrintf("data(%d) = %s\n", len, hex.c_str());
        #endif

        if(m_cmd.sendCommand(data.c_str()))
        {
            if(m_cmd.readResponse(REPLY_ANY, timeout))
            {
                ret = true;
            }
        }
    }
    m_cmd.readResponse(REPLY_IGNORE, oneSecond);

    return ret;
}

void Network::parseFilterResult(const char *strFilterResult)
{
    nbiot::string response = strFilterResult;
    size_t pos = response.find(',');
    if(nbiot::string::npos != pos)
    {
        nbiot::string number = response.substr((pos + 1));
        m_bytesAvail = static_cast<size_t>(atoi(number.c_str()));
        #ifdef DEBUG_MODEM
#ifdef DEBUG_COLOR
        debugPrintf("\n\033[0;32m[ MODEM    ]\033[0m ");
#endif
        debugPrintf("available bytes:: %d\r\n", m_bytesAvail);
        #endif
    }
}

bool Network::disconnect()
{
    bool ret = true;

    if(0 <= m_connectionNumber)
    {
        if(m_cmd.sendCommand(nbiot::string::Printf(cmdNSOCL_arg, m_connectionNumber)))
        {
            if(!m_cmd.readResponse(REPLY_OK, tenSeconds))
            {
                ret = false;
            }
        }
        else
        {
            ret = false;
        }
    }
    if(ret)
    {
        m_hostname = "";
        m_port = 0;
        m_connectionNumber = -1;
        m_cmd.removeUrcFilter(m_nsonmi.c_str());
        m_nsonmi = "";
    }
    m_cmd.readResponse(REPLY_IGNORE, oneSecond);

    return ret;
}
