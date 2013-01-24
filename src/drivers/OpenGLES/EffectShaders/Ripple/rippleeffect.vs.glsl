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

precision highp     float;

attribute vec4      inPosition;
attribute vec4      inDiffuse;
attribute vec2      inTextureCoordinate;

uniform   mat4      matViewProjection;
uniform   float     amplitude;
uniform   float     frequency;
uniform   float     time;
uniform   vec2      origin;

varying   vec2      textureCoordinate;
varying   vec4      diffuse;


void main()
{
   vec4  vertex = vec4(inPosition.xy, 0.0, 1.0);
   float dist   = distance(inPosition.xy, origin.xy);
   
   vertex.z += amplitude * sin( (dist*frequency) - time );
   vertex.y += amplitude * sin( (dist*frequency) - time );

   // Transform the vertex into screen space
   gl_Position = matViewProjection * vec4(vertex.xyz, 1.0);

   // Pass to fragment shader
   diffuse              = inDiffuse;
   textureCoordinate    = inTextureCoordinate;
}
