//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

precision highp    float;
attribute vec4     inPosition;
attribute vec4     inOpacity;
attribute vec2     inTextureCoordinate;

uniform   mat4     matViewProjection;
varying   vec2     textureCoordinate;
varying   vec4     opacity;

void main()
{
   // Transform the vertex into screen space
   gl_Position = matViewProjection * vec4(inPosition.xyz, 1.0);

   // Pass to fragment shader
   opacity              = inOpacity;
   textureCoordinate    = inTextureCoordinate;
}
