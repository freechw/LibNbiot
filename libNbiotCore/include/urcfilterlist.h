/* ========================================================================
 * LibNbiot: urcfilterlist.h
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expressed or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ========================================================================
*/

#ifndef URCFILTERLIST_H
#define URCFILTERLIST_H

#include <nbiotstring.h>
#include <list.h>
#include "FP.h"

namespace nbiot {

/*!
 * \brief urchandler
 */
typedef void (*urchandler)(const char*);

/*!
 * \brief The UrcFilter class
 */
class UrcFilter
{
public:
    /*!
     * \brief UrcFilter
     */
    UrcFilter() : urc(), handler() {}
    /*!
     * \brief UrcFilter
     * \param str
     */
    UrcFilter(const char* str) : urc(str), handler() {}
    /*!
     * \brief UrcFilter
     * \param str
     * \param urch
     */
    UrcFilter(const char* str, urchandler urch) : urc(str), handler()
    {
        handler.attach(urch);
    }
    /*!
     * \brief UrcFilter
     * \param other
     */
    UrcFilter(const UrcFilter& other) : urc(other.urc), handler(other.handler) {}

    virtual ~UrcFilter() {}

    /*!
     * \brief operator =
     * \param other
     * \return
     */
    UrcFilter& operator = (const UrcFilter& other)
    {
        urc = other.urc;
        handler = other.handler;
        return *this;
    }

    /*!
     * \brief operator ==
     * \param str
     * \return
     */
    bool operator == (const char* str) { return (urc == str); }
    /*!
     * \brief operator ==
     * \param str
     * \return
     */
    bool operator == (const nbiot::string& str) { return (0 == str.find(urc)); }

    template<class UHC>
    /*!
     * \brief setHandler
     * \param uhc
     */
    void setHandler(UHC* uhc, void (UHC::*urch)(const char*))
    {
        if(handler.attached())
        {
            handler.detach();
        }
        handler.attach(uhc, urch);
    }

    /*!
     * \brief setHandler
     * \param urch
     */
    void setHandler(urchandler urch)
    {
        if(handler.attached())
        {
            handler.detach();
        }
        handler.attach(urch);
    }

    /*!
     * \brief isValid
     * \return
     */
    bool isValid()
    {
        return ((!urc.empty()) && (handler.attached()));
    }

    /*!
     * \brief urc
     */
    string urc;
    /*!
     * \brief handler
     */
    FP<void, const char*> handler;
};

/*!
 * \brief The UrcFilterList class
 */
class UrcFilterList : public util::list<UrcFilter>
{
public:
    /*!
     * \brief indexOf
     * \param urcName
     * \return
     */
    int indexOf(const char* urcName) const
    {
        return util::list<UrcFilter>::indexOf(urcName);
    }

    /*!
     * \brief indexOf
     * \param urcName
     * \return
     */
    int indexOf(const nbiot::string& urcName) const
    {
        return util::list<UrcFilter>::indexOf(urcName);
    }
};

} // end of ns nbiot

#endif // URCFILTERLIST_H

