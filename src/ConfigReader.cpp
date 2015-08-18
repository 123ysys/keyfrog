/*********************************************************************************
 *   Copyright (C) 2006-2013 by Sebastian Gniazdowski                            *
 *   All Rights reserved.                                                        *
 *                                                                               *
 *   Redistribution and use in source and binary forms, with or without          *
 *   modification, are permitted provided that the following conditions          *
 *   are met:                                                                    *
 *   1. Redistributions of source code must retain the above copyright           *
 *      notice, this list of conditions and the following disclaimer.            *
 *   2. Redistributions in binary form must reproduce the above copyright        *
 *      notice, this list of conditions and the following disclaimer in the      *
 *      documentation and/or other materials provided with the distribution.     *
 *   3. Neither the name of the Keyfrog nor the names of its contributors        *
 *      may be used to endorse or promote products derived from this software    *
 *      without specific prior written permission.                               *
 *                                                                               *
 *   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND     *
 *   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE       *
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 *   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE    *
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL  *
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS     *
 *   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       *
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT  *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY   *
 *   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF      *
 *   SUCH DAMAGE.                                                                *
 *********************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "ConfigReader.h"
#include <exception>
#include <cstdlib>
#include "Debug.h"
#include "TermCode.h"

using namespace std;
using namespace keyfrog::TermCodes;

namespace keyfrog {

    /** 
     * Base initialization
     * 
     * @param configPath 
     */
    ConfigReader::ConfigReader() : m_config(NULL) {
        xmlKeepBlanksDefault(0);
    }

    /** 
     * Xml parser cleanup
     */
    ConfigReader::~ConfigReader() {
        xmlCleanupParser();
    }

    /** 
     * Uses sub-function for each bigger group of tags.
     */
    void ConfigReader::readConfig() {
        configXmlDoc = xmlReadFile(m_config->configPath().c_str(), NULL, 0);
        if(configXmlDoc == NULL) {
            _inf(
                    "%sThere is no config file (~/.keyfrog/config).\n"
                    "You must create one. You can use example config included with keyfrog.\n"
                    "It should be placed at `/usr/share/keyfrog/doc/sample-config' or\n"
                    "`/usr/local/share/keyfrog/doc/sample-config'. If it's not there,\n"
                    "download sources and check doc/ directory.%s\n"
                    , ccyan, creset);
        }
        xmlNode *rootElement = xmlDocGetRootElement(configXmlDoc);
        // Walk through nodes and invoke proper processing functions
        for (xmlNode * cur_node = rootElement->children; cur_node; cur_node = cur_node->next) {
            if (cur_node->type != XML_ELEMENT_NODE) {
                continue;
            }
            if(0==xmlStrcmp((const xmlChar *)"application-groups",cur_node->name)) {
                processGroups(cur_node->children);
            }
            if(0==xmlStrcmp((const xmlChar *)"options",cur_node->name)) {
                processOptions(cur_node->children);
            }
        }
        xmlFreeDoc(configXmlDoc);
    }

    /**
     * Processes groups and puts them into FilterConfig
     * @param startGroup First group to process
     */
    void ConfigReader::processGroups(xmlNode *startGroup) {
        for (xmlNode * cur_group = startGroup; cur_group; cur_group = cur_group->next) {
            if(cur_group->type == XML_COMMENT_NODE) {
                continue;
            }
            if(0!=xmlStrcmp((const xmlChar *)"group",cur_group->name)) {
                throw exception();
            }
            xmlChar *groupId = xmlGetProp(cur_group, (const xmlChar *)"id");
            Group newGroup;
            newGroup.setId(atoi((char *)groupId));
            // Walk through rules of this group
            for(xmlNode * cur_rule = cur_group->children; cur_rule; cur_rule = cur_rule->next) {
                if (cur_rule->type != XML_ELEMENT_NODE) {
                    continue;
                }
                if(0!=xmlStrcmp((const xmlChar *)"rule",cur_rule->name)) {
                    throw exception();
                }
                xmlChar *ruleType = xmlGetProp(cur_rule, (const xmlChar *)"type");
                // WND_CLASS X11 window property
                if(0==xmlStrcmp((const xmlChar *)"windowClassName", ruleType)) {
                    // It contains WND_CLASS window property
                    // There must be only one children with text
                    if(cur_rule->children == NULL || 
                            cur_rule->children->type != XML_TEXT_NODE ||
                            cur_rule->children->next != NULL ||
                            cur_rule->children->children != NULL) 
                    {
                        // TODO: descriptful error message instead of exception throw
                        _dbg("exception1");
                        throw exception();
                    }
                    const char * tmp = (const char *)cur_rule->children->content;
                    newGroup.addWindowClass(tmp);
                    _dbg("new wndClass %s%s%s", cboldGreen, tmp, creset);
                } else if(0==xmlStrcmp((const xmlChar *)"terminalProcessName", ruleType)) {
                    if(cur_rule->children == NULL || 
                            cur_rule->children->type != XML_TEXT_NODE ||
                            cur_rule->children->next != NULL ||
                            cur_rule->children->children != NULL) 
                    {
                        _dbg("exception2");
                        throw exception();
                    }
                    const char * tmp = (const char *)cur_rule->children->content;
                    newGroup.addTerminalProcess(tmp);
                    _dbg("new termProc %s-T-%s%s%s", cboldRed, cboldGreen, tmp, creset);
                } else if(0==xmlStrcmp((const xmlChar *)"processName", ruleType)) {
                    if(cur_rule->children == NULL || 
                            cur_rule->children->type != XML_TEXT_NODE ||
                            cur_rule->children->next != NULL ||
                            cur_rule->children->children != NULL) 
                    {
                        _dbg("exception3");
                        throw exception();
                    }
                    const char * tmp = (const char *)cur_rule->children->content;
                    newGroup.addProcess(tmp);
                    _dbg("new proc %s%s%s", cboldGreen, tmp, creset);
                }
            }
            // XXX: should empty group be added?
            m_config->filterConfig().addGroup(newGroup);
            _dbg("group %s%d%s addGroup", cboldYellow, newGroup.id(), creset);
        }
    }

    void ConfigReader::processOptions(xmlNode *startOption) {
        for (xmlNode * cur_opt = startOption; cur_opt; cur_opt = cur_opt->next) {
            char *_opt;
            string opt;

            if( 0 == xmlStrcmp((const xmlChar *)"debug", cur_opt->name) ) {
                // state=""
                _opt = (char *) xmlGetProp(cur_opt, (const xmlChar *)"state");
                opt = _opt ? _opt : "off";
                if(opt == "off")
                    m_config->options().setDebugState(false);
                else if(opt == "on")
                    m_config->options().setDebugState(true);

                // uselog=""
                _opt = (char *) xmlGetProp(cur_opt, (const xmlChar *)"uselogfile");
                opt = _opt ? _opt : "off";
                if(opt == "off")
                    m_config->options().setDebugUseLogFile(false);
                else if(opt == "on")
                    m_config->options().setDebugUseLogFile(true);

                // usestderr=""
                _opt = (char *) xmlGetProp(cur_opt, (const xmlChar *)"usestderr");
                opt = _opt ? _opt : "off";
                if(opt == "off")
                    m_config->options().setDebugUseStdErr(false);
                else if(opt == "on")
                    m_config->options().setDebugUseStdErr(true);

                // logfile=""
                _opt = (char *) xmlGetProp(cur_opt, (const xmlChar *)"logfile");
                if(_opt) {
                    opt = _opt;
                    m_config->options().setDebugLogFile(opt);
                }

            } else if( 0 == xmlStrcmp((const xmlChar *)"cluster", cur_opt->name) ) {
                // cluster=""
                _opt = (char *) xmlGetProp(cur_opt, (const xmlChar *)"size");
                if(_opt) {
                    int size = atoi(_opt);
                    m_config->options().setClusterSize(size);
                }
            } else {
                // skip
            }

        }
    }

}
