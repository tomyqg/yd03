﻿/*
 * cms.h : GeeXboX uShare Connection Management Service header.
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef RCS_H_
#define RCS_H_

#define RCS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
    "<specVersion>" \
        "<major>1</major>" \
        "<minor>0</minor>" \
    "</specVersion>" \
    "<actionList>" \
        "<action>" \
            "<name>GetMute</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>CurrentMute</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>Mute</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>GetVolume</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>CurrentVolume</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>Volume</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>GetVolumeDB</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>CurrentVolume</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>VolumeDB</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>GetVolumeDBRange</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>MinValue</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>VolumeDB</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>MaxValue</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>VolumeDB</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>ListPresets</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>CurrentPresetNameList</name>" \
                    "<direction>out</direction>" \
                    "<relatedStateVariable>PresetNameList</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>SelectPreset</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>PresetName</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_PresetName</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>SetMute</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>DesiredMute</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>Mute</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
        "<action>" \
            "<name>SetVolume</name>" \
            "<argumentList>" \
                "<argument>" \
                    "<name>InstanceID</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>Channel</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>" \
                "</argument>" \
                "<argument>" \
                    "<name>DesiredVolume</name>" \
                    "<direction>in</direction>" \
                    "<relatedStateVariable>Volume</relatedStateVariable>" \
                "</argument>" \
            "</argumentList>" \
        "</action>" \
    "</actionList>" \
    "<serviceStateTable>" \
        "<stateVariable sendEvents=\"yes\">" \
            "<name>LastChange</name>" \
            "<dataType>string</dataType>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>A_ARG_TYPE_Channel</name>" \
            "<dataType>string</dataType>" \
            "<allowedValueList>" \
                "<allowedValue>Master</allowedValue>" \
            "</allowedValueList>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>A_ARG_TYPE_InstanceID</name>" \
            "<dataType>ui4</dataType>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>Volume</name>" \
            "<dataType>ui2</dataType>" \
            "<allowedValueRange>" \
                "<minimum>1</minimum>" \
                "<maximum>100</maximum>" \
                "<step>5</step>" \
            "</allowedValueRange>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>Mute</name>" \
            "<dataType>boolean</dataType>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>PresetNameList</name>" \
            "<dataType>string</dataType>" \
            "<allowedValueList>" \
                "<allowedValue>FactoryDefaults</allowedValue>" \
            "</allowedValueList>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>A_ARG_TYPE_PresetName</name>" \
            "<dataType>string</dataType>" \
            "<allowedValueList>" \
                "<allowedValue>FactoryDefaults</allowedValue>" \
            "</allowedValueList>" \
        "</stateVariable>" \
        "<stateVariable sendEvents=\"no\">" \
            "<name>VolumeDB</name>" \
            "<dataType>i2</dataType>" \
            "<allowedValueRange>" \
                "<minimum>-32767</minimum>" \
                "<maximum>32767</maximum>" \
            "</allowedValueRange>" \
        "</stateVariable>" \
    "</serviceStateTable>" \
"</scpd>"
#define RCS_DESCRIPTION_LEN strlen (RCS_DESCRIPTION)

#define RCS_LOCATION "/web/rcs.xml"

#define RCS_SERVICE_ID "urn:upnp-org:serviceId:RenderingControl"
#define RCS_SERVICE_TYPE "urn:schemas-upnp-org:service:RenderingControl:1"

struct rcs_t {
    int mute;
    int volume;
    int volumeDB;
};

#endif /* RCS_H_ */
