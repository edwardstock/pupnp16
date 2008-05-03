///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation 
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met: 
//
// * Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
// * Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
// * Neither name of Intel Corporation nor the names of its contributors 
// may be used to endorse or promote products derived from this software 
// without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include "sample_util.h"
#include <stdarg.h>

#if !UPNP_HAVE_TOOLS
#	error "Need upnptools.h to compile samples ; try ./configure --enable-tools"
#endif


/*
   Function pointer to use for displaying formatted
   strings. Set on Initialization of device. 
 */
int initialize = 1;
print_string gPrintFun = NULL;
state_update gStateUpdateFun = NULL;

//mutex to control displaying of events
ithread_mutex_t display_mutex;

/********************************************************************************
 * SampleUtil_Initialize
 *
 * Description: 
 *     Initializes the sample util. Must be called before any sample util 
 *     functions. May be called multiple times.
 *
 * Parameters:
 *   print_function - print function to use in SampleUtil_Print
 *
 ********************************************************************************/
int
SampleUtil_Initialize( print_string print_function )
{
    if( initialize ) {
        ithread_mutexattr_t attr;

        gPrintFun = print_function;
        ithread_mutexattr_init( &attr );
        ithread_mutexattr_setkind_np( &attr, ITHREAD_MUTEX_RECURSIVE_NP );
        ithread_mutex_init( &display_mutex, &attr );
        ithread_mutexattr_destroy( &attr );
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_RegisterUpdateFunction
 *
 * Description: 
 *
 * Parameters:
 *
 ********************************************************************************/
int
SampleUtil_RegisterUpdateFunction( state_update update_function )
{
    static int initialize = 1;  //only intialize once

    if( initialize ) {
        gStateUpdateFun = update_function;
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_Finish
 *
 * Description: 
 *     Releases Resources held by sample util.
 *
 * Parameters:
 *
 ********************************************************************************/
int
SampleUtil_Finish()
{
    ithread_mutex_destroy( &display_mutex );
    gPrintFun = NULL;
    initialize = 1;
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_GetElementValue
 *
 * Description: 
 *       Given a DOM node such as <Channel>11</Channel>, this routine
 *       extracts the value (e.g., 11) from the node and returns it as 
 *       a string. The string must be freed by the caller using 
 *       free.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *
 ********************************************************************************/

char *
SampleUtil_GetElementValue( IN IXML_Element * element )
{

    IXML_Node *child = ixmlNode_getFirstChild( ( IXML_Node * ) element );

    char *temp = NULL;

    if( ( child != 0 ) && ( ixmlNode_getNodeType( child ) == eTEXT_NODE ) ) {
        temp = strdup( ixmlNode_getNodeValue( child ) );
    }

    return temp;
}

/********************************************************************************
 * SampleUtil_GetFirstServiceList
 *
 * Description: 
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the first service list
 *       (i.e., the service list for the root device).  The service list
 *       is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the service list
 *
 ********************************************************************************/
IXML_NodeList *
SampleUtil_GetFirstServiceList( IN IXML_Document * doc )
{
    IXML_NodeList *ServiceList = NULL;
    IXML_NodeList *servlistnodelist = NULL;
    IXML_Node *servlistnode = NULL;

    servlistnodelist =
        ixmlDocument_getElementsByTagName( doc, "serviceList" );
    if( servlistnodelist && ixmlNodeList_length( servlistnodelist ) ) {

        /*
           we only care about the first service list, from the root device 
         */
        servlistnode = ixmlNodeList_item( servlistnodelist, 0 );

        /*
           create as list of DOM nodes 
         */
        ServiceList =
            ixmlElement_getElementsByTagName( ( IXML_Element * )
                                              servlistnode, "service" );
    }

    if( servlistnodelist )
        ixmlNodeList_free( servlistnodelist );

    return ServiceList;
}

/********************************************************************************
 * SampleUtil_GetFirstDocumentItem
 *
 * Description: 
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       String must be freed by caller using free.
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
SampleUtil_GetFirstDocumentItem( IN IXML_Document * doc,
                                 IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

    if( nodeList ) {
        if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
            textNode = ixmlNode_getFirstChild( tmpNode );

            ret = strdup( ixmlNode_getNodeValue( textNode ) );
        }
    }

    if( nodeList )
        ixmlNodeList_free( nodeList );
    return ret;
}

/********************************************************************************
 * SampleUtil_GetFirstElementItem
 *
 * Description: 
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       The string must be freed using free.
 * Parameters:
 *   node -- The DOM element from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
SampleUtil_GetFirstElementItem( IN IXML_Element * element,
                                IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlElement_getElementsByTagName( element, ( char * )item );

    if( nodeList == NULL ) {
        SampleUtil_Print( "Error finding %s in XML Node\n", item );
        return NULL;
    }

    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) == NULL ) {
        SampleUtil_Print( "Error finding %s value in XML Node\n", item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    textNode = ixmlNode_getFirstChild( tmpNode );

    ret = strdup( ixmlNode_getNodeValue( textNode ) );

    if( !ret ) {
        SampleUtil_Print( "Error allocating memory for %s in XML Node\n",
                          item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    ixmlNodeList_free( nodeList );

    return ret;
}

/********************************************************************************
 * SampleUtil_PrintEventType
 *
 * Description: 
 *       Prints a callback event type as a string.
 *
 * Parameters:
 *   S -- The callback event
 *
 ********************************************************************************/
void
SampleUtil_PrintEventType( IN Upnp_EventType S )
{
    switch ( S ) {

        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
            SampleUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n" );
            break;
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
            SampleUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_RESULT:
            SampleUtil_Print( "UPNP_DISCOVERY_SEARCH_RESULT\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            SampleUtil_Print( "UPNP_DISCOVERY_SEARCH_TIMEOUT\n" );
            break;

            /*
               SOAP Stuff 
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            SampleUtil_Print( "UPNP_CONTROL_ACTION_REQUEST\n" );
            break;
        case UPNP_CONTROL_ACTION_COMPLETE:
            SampleUtil_Print( "UPNP_CONTROL_ACTION_COMPLETE\n" );
            break;
        case UPNP_CONTROL_GET_VAR_REQUEST:
            SampleUtil_Print( "UPNP_CONTROL_GET_VAR_REQUEST\n" );
            break;
        case UPNP_CONTROL_GET_VAR_COMPLETE:
            SampleUtil_Print( "UPNP_CONTROL_GET_VAR_COMPLETE\n" );
            break;

            /*
               GENA Stuff 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIPTION_REQUEST\n" );
            break;
        case UPNP_EVENT_RECEIVED:
            SampleUtil_Print( "UPNP_EVENT_RECEIVED\n" );
            break;
        case UPNP_EVENT_RENEWAL_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_RENEWAL_COMPLETE\n" );
            break;
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIBE_COMPLETE\n" );
            break;
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n" );
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
            SampleUtil_Print( "UPNP_EVENT_AUTORENEWAL_FAILED\n" );
            break;
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIPTION_EXPIRED\n" );
            break;

    }
}

/********************************************************************************
 * SampleUtil_PrintEvent
 *
 * Description: 
 *       Prints callback event structure details.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- The callback event structure
 *
 ********************************************************************************/
int
SampleUtil_PrintEvent( IN Upnp_EventType EventType,
                       IN void *Event )
{

    ithread_mutex_lock( &display_mutex );

    SampleUtil_Print
        ( "\n\n\n======================================================================\n" );
    SampleUtil_Print
        ( "----------------------------------------------------------------------\n" );
    SampleUtil_PrintEventType( EventType );

    switch ( EventType ) {

            /*
               SSDP 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
            {
                UpnpDiscovery *d_event = (UpnpDiscovery *)Event;
                SampleUtil_Print(
                    "ErrCode     =  %d\n"
                    "Expires     =  %d\n"
                    "DeviceId    =  %s\n"
                    "DeviceType  =  %s\n"
                    "ServiceType =  %s\n"
                    "ServiceVer  =  %s\n"
                    "Location    =  %s\n"
                    "OS          =  %s\n"
                    "Date        =  %s\n"
                    "Ext         =  %s\n",
                    UpnpDiscovery_get_ErrCode(d_event),
                    UpnpDiscovery_get_Expires(d_event),
                    UpnpString_get_String(UpnpDiscovery_get_DeviceID(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_DeviceType(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_ServiceType(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_ServiceVer(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_Location(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_Os(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_Date(d_event)),
                    UpnpString_get_String(UpnpDiscovery_get_Ext(d_event)));
            }
            break;

        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            // Nothing to print out here
            break;

            /*
               SOAP 
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            {
                UpnpActionRequest *a_event = (UpnpActionRequest *)Event;
		IXML_Document *actionRequestDoc = NULL;
		IXML_Document *actionResultDoc = NULL;
                char *xmlbuff = NULL;

                SampleUtil_Print(
			"ErrCode     =  %d\n"
			"ErrStr      =  %s\n"
			"ActionName  =  %s\n"
			"UDN         =  %s\n"
			"ServiceID   =  %s\n",
			UpnpActionRequest_get_ErrCode(a_event),
			UpnpString_get_String(UpnpActionRequest_get_ErrStr(a_event)),
			UpnpString_get_String(UpnpActionRequest_get_ActionName(a_event)),
			UpnpString_get_String(UpnpActionRequest_get_DevUDN(a_event)),
			UpnpString_get_String(UpnpActionRequest_get_ServiceID(a_event)));

		actionRequestDoc = UpnpActionRequest_get_ActionRequest(a_event);
                if ( actionRequestDoc ) {
                    xmlbuff = ixmlPrintNode( (IXML_Node *)actionRequestDoc );
                    if ( xmlbuff ) {
                        SampleUtil_Print( "ActRequest  =  %s\n", xmlbuff );
                        ixmlFreeDOMString( xmlbuff );
                    }
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActRequest  =  (null)\n" );
                }
                actionResultDoc = UpnpActionRequest_get_ActionResult(a_event);
                if ( actionResultDoc ) {
                    xmlbuff = ixmlPrintNode( (IXML_Node *)actionResultDoc );
                    if ( xmlbuff ) {
                        SampleUtil_Print( "ActResult   =  %s\n", xmlbuff );
                        ixmlFreeDOMString( xmlbuff );
		    }
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActResult   =  (null)\n" );
                }
            }
            break;

        case UPNP_CONTROL_ACTION_COMPLETE:
            {
                UpnpActionComplete *a_event = (UpnpActionComplete *)Event;
                char *xmlbuff = NULL;
		int errCode = UpnpActionComplete_get_ErrCode(a_event);
		const char *ctrlURL = UpnpString_get_String(
                    UpnpActionComplete_get_CtrlUrl(a_event));
		IXML_Document *actionRequest =
                    UpnpActionComplete_get_ActionRequest(a_event);
		IXML_Document *actionResult =
                    UpnpActionComplete_get_ActionResult(a_event);

                SampleUtil_Print(
                    "ErrCode     =  %d\n"
                    "CtrlUrl     =  %s\n",
                    errCode, ctrlURL);
                if( actionRequest ) {
                    xmlbuff = ixmlPrintNode((IXML_Node *)actionRequest);
                    if( xmlbuff ) {
                        SampleUtil_Print( "ActRequest  =  %s\n", xmlbuff );
                        ixmlFreeDOMString( xmlbuff );
		    }
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActRequest  =  (null)\n" );
                }

                if( actionResult ) {
                    xmlbuff = ixmlPrintNode((IXML_Node *)actionResult);
                    if( xmlbuff ) {
                        SampleUtil_Print( "ActResult   =  %s\n", xmlbuff );
                        ixmlFreeDOMString( xmlbuff );
		    }
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActResult   =  (null)\n" );
                }
            }
            break;

        case UPNP_CONTROL_GET_VAR_REQUEST:
            {
                UpnpStateVarRequest *sv_event = (UpnpStateVarRequest *)Event;
                SampleUtil_Print(
                    "ErrCode     =  %d\n"
                    "ErrStr      =  %s\n"
                    "UDN         =  %s\n"
                    "ServiceID   =  %s\n"
                    "StateVarName=  %s\n"
                    "CurrentVal  =  %s\n",
                    UpnpStateVarRequest_get_ErrCode(sv_event),
                    UpnpString_get_String(UpnpStateVarRequest_get_ErrStr(sv_event)),
                    UpnpString_get_String(UpnpStateVarRequest_get_DevUDN(sv_event)),
                    UpnpString_get_String(UpnpStateVarRequest_get_ServiceID(sv_event)),
                    UpnpString_get_String(UpnpStateVarRequest_get_StateVarName(sv_event)),
                    UpnpStateVarRequest_get_CurrentVal(sv_event));
            }
            break;

        case UPNP_CONTROL_GET_VAR_COMPLETE:
            {
                UpnpStateVarComplete *sv_event = (UpnpStateVarComplete *)Event;
                SampleUtil_Print(
                    "ErrCode     =  %d\n"
                    "CtrlUrl     =  %s\n"
                    "StateVarName=  %s\n"
                    "CurrentVal  =  %s\n",
                    UpnpStateVarComplete_get_ErrCode(sv_event),
                    UpnpString_get_String(UpnpStateVarComplete_get_CtrlUrl(sv_event)),
                    UpnpString_get_String(UpnpStateVarComplete_get_StateVarName(sv_event)),
                    UpnpStateVarComplete_get_CurrentVal(sv_event));
            }
            break;

            /*
               GENA 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            {
                struct Upnp_Subscription_Request *sr_event =
                    (struct Upnp_Subscription_Request *)Event;
                SampleUtil_Print(
                    "ServiceID   =  %s\n"
                    "UDN         =  %s\n"
                    "SID         =  %s\n",
                    sr_event->ServiceId,
                    sr_event->UDN,
                    sr_event->Sid );
            }
            break;

        case UPNP_EVENT_RECEIVED:
            {
                UpnpEvent *e_event = (UpnpEvent *)Event;
                char *xmlbuff = NULL;
                xmlbuff = ixmlPrintNode(
			(IXML_Node *) UpnpEvent_get_ChangedVariables(e_event));
                SampleUtil_Print(
                    "SID         =  %s\n"
                    "EventKey    =  %d\n"
                    "ChangedVars =  %s\n",
                    UpnpString_get_String(UpnpEvent_get_SID(e_event)),
                    UpnpEvent_get_EventKey(e_event),
                    xmlbuff);
                ixmlFreeDOMString(xmlbuff);
            }
            break;

        case UPNP_EVENT_RENEWAL_COMPLETE:
            {
                EventSubscribe *es_event = (EventSubscribe *)Event;
                SampleUtil_Print(
                    "SID         =  %s\n"
                    "ErrCode     =  %d\n"
                    "TimeOut     =  %d\n",
                    UpnpString_get_String(UpnpEventSubscribe_get_SID(es_event)),
                    UpnpEventSubscribe_get_ErrCode(es_event),
                    UpnpEventSubscribe_get_TimeOut(es_event));
            }
            break;

        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            {
                EventSubscribe *es_event = (EventSubscribe *)Event;
                SampleUtil_Print(
                    "SID         =  %s\n"
                    "ErrCode     =  %d\n"
                    "PublisherURL=  %s\n"
                    "TimeOut     =  %d\n",
                    UpnpString_get_String(UpnpEventSubscribe_get_SID(es_event)),
                    UpnpEventSubscribe_get_ErrCode(es_event),
                    UpnpString_get_String(UpnpEventSubscribe_get_PublisherUrl(es_event)),
                    UpnpEventSubscribe_get_TimeOut(es_event));
            }
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                EventSubscribe *es_event = (EventSubscribe *)Event;
                SampleUtil_Print(
                    "SID         =  %s\n"
                    "ErrCode     =  %d\n"
                    "PublisherURL=  %s\n"
                    "TimeOut     =  %d\n",
                    UpnpString_get_String(UpnpEventSubscribe_get_SID(es_event)),
                    UpnpEventSubscribe_get_ErrCode(es_event),
                    UpnpString_get_String(UpnpEventSubscribe_get_PublisherUrl(es_event)),
                    UpnpEventSubscribe_get_TimeOut(es_event));
            }
            break;

    }
    SampleUtil_Print(
        "----------------------------------------------------------------------\n"
        "======================================================================\n\n\n\n" );

    ithread_mutex_unlock( &display_mutex );
    return 0;
}

/********************************************************************************
 * SampleUtil_FindAndParseService
 *
 * Description: 
 *       This routine finds the first occurance of a service in a DOM representation
 *       of a description document and parses it.  
 *
 * Parameters:
 *   DescDoc -- The DOM description document
 *   location -- The location of the description document
 *   serviceSearchType -- The type of service to search for
 *   serviceId -- OUT -- The service ID
 *   eventURL -- OUT -- The event URL for the service
 *   controlURL -- OUT -- The control URL for the service
 *
 ********************************************************************************/
int
SampleUtil_FindAndParseService( IN IXML_Document * DescDoc,
                                IN const char *location,
                                IN char *serviceType,
                                OUT char **serviceId,
                                OUT char **eventURL,
                                OUT char **controlURL )
{
    int i,
      length,
      found = 0;
    int ret;
    char *tempServiceType = NULL;
    char *baseURL = NULL;
    const char *base = NULL;
    char *relcontrolURL = NULL,
     *releventURL = NULL;
    IXML_NodeList *serviceList = NULL;
    IXML_Element *service = NULL;

    baseURL = SampleUtil_GetFirstDocumentItem( DescDoc, "URLBase" );
    if (baseURL) {
        base = baseURL;
    } else {
        base = location;
    }

    serviceList = SampleUtil_GetFirstServiceList( DescDoc );
    length = ixmlNodeList_length( serviceList );
    for( i = 0; i < length; i++ ) {
        service = ( IXML_Element * ) ixmlNodeList_item( serviceList, i );
        tempServiceType =
            SampleUtil_GetFirstElementItem( ( IXML_Element * ) service,
                                            "serviceType" );

        if( strcmp( tempServiceType, serviceType ) == 0 ) {
            SampleUtil_Print( "Found service: %s\n", serviceType );
            *serviceId =
                SampleUtil_GetFirstElementItem( service, "serviceId" );
            SampleUtil_Print( "serviceId: %s\n", ( *serviceId ) );
            relcontrolURL =
                SampleUtil_GetFirstElementItem( service, "controlURL" );
            releventURL =
                SampleUtil_GetFirstElementItem( service, "eventSubURL" );

            *controlURL =
                malloc( strlen( base ) + strlen( relcontrolURL ) + 1 );
            if( *controlURL ) {
                ret = UpnpResolveURL( base, relcontrolURL, *controlURL );
                if( ret != UPNP_E_SUCCESS )
                    SampleUtil_Print
                        ( "Error generating controlURL from %s + %s\n",
                          base, relcontrolURL );
            }

            *eventURL =
                malloc( strlen( base ) + strlen( releventURL ) + 1 );
            if( *eventURL ) {
                ret = UpnpResolveURL( base, releventURL, *eventURL );
                if( ret != UPNP_E_SUCCESS )
                    SampleUtil_Print
                        ( "Error generating eventURL from %s + %s\n", base,
                          releventURL );
            }

            if( relcontrolURL )
                free( relcontrolURL );
            if( releventURL )
                free( releventURL );
            relcontrolURL = releventURL = NULL;

            found = 1;
            break;
        }

        if( tempServiceType )
            free( tempServiceType );
        tempServiceType = NULL;
    }

    if( tempServiceType )
        free( tempServiceType );
    if( serviceList )
        ixmlNodeList_free( serviceList );
    if( baseURL )
        free( baseURL );

    return ( found );
}

// printf wrapper for portable code
int
uprint1( char *fmt,
         ... )
{
    va_list ap;
    char *buf = NULL;
    int size;

    va_start( ap, fmt );
    size = vsnprintf( buf, 0, fmt, ap );
    va_end( ap );

    if( size > 0 ) {
        buf = ( char * )malloc( size + 1 );
        if( vsnprintf( buf, size + 1, fmt, ap ) != size ) {
            free( buf );
            buf = NULL;
        }
    }

    if( buf ) {
        ithread_mutex_lock( &display_mutex );
        if( gPrintFun )
            gPrintFun( buf );
        ithread_mutex_unlock( &display_mutex );
        free( buf );
    }

    return size;
}

/********************************************************************************
 * SampleUtil_Print
 *
 * Description: 
 *      Provides platform-specific print functionality.  This function should be
 *      called when you want to print content suitable for console output (i.e.,
 *      in a large text box or on a screen).  If your device/operating system is 
 *      not supported here, you should add a port.
 *
 * Parameters:
 *		Same as printf()
 *
 ********************************************************************************/
int SampleUtil_Print(char *fmt, ...)
{
#define MAX_BUF 1024
    va_list ap;
    static char buf[MAX_BUF];
    int rc;

    /* Protect both the display and the static buffer with the mutex */
    ithread_mutex_lock(&display_mutex);
    va_start(ap, fmt);
    rc = vsnprintf(buf, MAX_BUF, fmt, ap);
    va_end(ap);

    if (gPrintFun) {
        gPrintFun(buf);
    }
    ithread_mutex_unlock(&display_mutex);

    return rc;
}

/********************************************************************************
 * SampleUtil_StateUpdate
 *
 * Description: 
 *
 * Parameters:
 *
 ********************************************************************************/
void
SampleUtil_StateUpdate( const char *varName,
                        const char *varValue,
                        const char *UDN,
                        eventType type )
{
    if( gStateUpdateFun )
        gStateUpdateFun( varName, varValue, UDN, type );
    // TBD: Add mutex here?
}
