@echo off

set WSDL2H_BIN=wsdl2h.exe

set DST=onvif_head\onvif.h

%WSDL2H_BIN% -c++11 -x -t gsoap\typemap.dat -o %DST%  https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl   https://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl https://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl  https://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl  https://www.onvif.org/onvif/ver10/deviceio.wsdl  https://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl  https://www.onvif.org/onvif/ver20/media/wsdl/media.wsdl

:: # https://www.onvif.org/ver10/events/wsdl/event.wsdl \

:: # http://www.onvif.org/onvif/ver10/display.wsdl \

:: # http://www.onvif.org/onvif/ver10/deviceio.wsdl \

:: # http://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl \

:: # http://www.onvif.org/onvif/ver10/receiver.wsdl \

:: # http://www.onvif.org/onvif/ver10/recording.wsdl \

:: # http://www.onvif.org/onvif/ver10/search.wsdl \

:: # http://www.onvif.org/onvif/ver10/replay.wsdl \

:: # http://www.onvif.org/onvif/ver20/analytics/wsdl/analytics.wsdl \

:: # http://www.onvif.org/onvif/ver10/analyticsdevice.wsdl \

:: # http://www.onvif.org/onvif/ver10/schema/onvif.xsd \

:: # http://www.onvif.org/ver10/actionengine.wsdl \

:: # http://www.onvif.org/ver10/pacs/accesscontrol.wsdl \

:: # http://www.onvif.org/ver10/pacs/doorcontrol.wsdl \

:: # http://www.onvif.org/ver10/advancedsecurity/wsdl/advancedsecurity.wsdl \

:: # http://www.onvif.org/ver10/accessrules/wsdl/accessrules.wsdl \

:: # http://www.onvif.org/ver10/credential/wsdl/credential.wsdl \

:: # http://www.onvif.org/ver10/schedule/wsdl/schedule.wsdl \

:: # http://www.onvif.org/ver10/pacs/types.xsd

pause